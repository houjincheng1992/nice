#include "service.h"

#include <string>
#include <vector>
#include <boost/algorithm/string/join.hpp>

#include <butil/time.h>
#include "xls.h"

#include "utils/logger.h"

namespace nicer {
void NicerService::check_excel_status(
                        ::google::protobuf::RpcController* controller,
                        const ::nicer::HttpRequest* request,
                        ::nicer::HttpResponse* response,
                        ::google::protobuf::Closure* done) {
    struct timeval tv, tv1;
    gettimeofday(&tv,NULL);
    brpc::ClosureGuard guard(done);
    brpc::Controller* ctrl = static_cast<brpc::Controller*>(controller);

    std::string str = ctrl->request_attachment().to_string();

    xls::xls_error_t error = xls::LIBXLS_OK;
    xls::xlsWorkBook *wb = xls::xls_open_buffer((const unsigned char*)str.c_str(), str.size(), "UTF-8", &error);
    if (wb == NULL) {
        printf("Error reading file: %s\n", xls_getError(error));
        exit(1);
    }

    std::vector<std::string> titles;
    std::vector<std::string> datas;
    std::vector<std::string> err_msg;
    std::string msg;
    for (int32_t i = 0; i < wb->sheets.count; i++) { // sheets
        xls::xlsWorkSheet *work_sheet = xls::xls_getWorkSheet(wb, i);
        error = xls::xls_parseWorkSheet(work_sheet);
        // rows
        for (int32_t j = 0; j <= work_sheet->rows.lastrow; j++) {
            xls::xlsRow *row = xls::xls_row(work_sheet, j);

            int32_t cols_num = work_sheet->rows.lastcol;
            if (j == 0 && cols_num == 0) {
                break;
            }

            // columns
            if (cols_num > titles.size()) {
                msg = "第" + std::to_string(j + 1) + "行数据的列数超出标题列数，请检查";
                err_msg.emplace_back(msg);
            }

            for (int32_t k = 0; k <= cols_num; k++) {
                xls::xlsCell *cell = &row->cells.cell[k];
                if (j == 0 && cell->str != NULL) {
                    titles.emplace_back(cell->str);
                    continue;
                } else if (j == 0) {
                    err_msg.emplace_back("标题格式有误");
                    break;
                } else if (cell->str != NULL) {
                    datas.emplace_back(cell->str);
                } else {
                    msg = "第" + std::to_string(j + 1) + "行第" + std::to_string(k + 1) + "列数据异常，请检查";
                    err_msg.emplace_back(msg);
                }
                
            }
        }
        xls::xls_close_WS(work_sheet);
    }
    xls::xls_close_WB(wb);

    ctrl->http_response().set_content_type("application/json;charset=utf-8");
    std::string response_str = "{\"msg\":\"done\"}";
    if (err_msg.size()) {
        response_str = "{\"msg\":\"" + boost::algorithm::join(err_msg, ";") +"\"}";
    }
    ctrl->response_attachment().append(response_str);
    gettimeofday(&tv1,NULL);
    INFLOG << "gettimeofday cost: " << (tv1.tv_sec - tv.tv_sec)*1000.0 + (tv1.tv_usec - tv.tv_usec)/1000.0<< "ms";
}
}
