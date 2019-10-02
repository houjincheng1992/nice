#include "service.h"

#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/join.hpp>

#include <butil/time.h>
#include "xls.h"

#include "utils/logger.h"

namespace nicer {

static const std::set<std::string> required_field = {
    "年级编号", "班级编号", "班级名称", "学籍号", "民族代码", "姓名", "性别", "出生日期", "家庭住址"
};

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

    std::vector<std::string> err_msg;
    std::string msg;
    for (int32_t i = 0; i < wb->sheets.count; i++) { // sheets
        xls::xlsWorkSheet *work_sheet = xls::xls_getWorkSheet(wb, i);
        error = xls::xls_parseWorkSheet(work_sheet);
        // rows
        int32_t rows_num = work_sheet->rows.lastrow;
        int32_t cols_num = work_sheet->rows.lastcol;

        if (rows_num <= 1) {
            continue;
        }

        std::vector<std::string> titles;    // 标题数组
        std::map<std::string, int32_t> title_index;     // 标题与位置对应关系
        std::vector<std::vector<std::string>> datas;
        xls::xlsRow *row = xls::xls_row(work_sheet, 0);
        for (int32_t col = 0; col < cols_num; ++col) {
            xls::xlsCell *cell = &row->cells.cell[col];
            if (cell->str == NULL) {
                err_msg.emplace_back("请检查标题");
                break;
            }
            titles.emplace_back(cell->str);
            title_index.emplace(cell->str, col);
        }

        // 会有几个sheet呢? 此处可能会有bug
        if (err_msg.size()) {
            continue;
        }

        for (int32_t j = 1; j < rows_num; j++) {
            xls::xlsRow *row = xls::xls_row(work_sheet, j);

            // columns
            if (cols_num > titles.size()) {
                msg = "第" + std::to_string(j + 1) + "行数据信息项目异常。";
                err_msg.emplace_back(msg);
                continue;
            }

            for (int32_t k = 0; k < cols_num; k++) {
                xls::xlsCell *cell = &row->cells.cell[k];
                if (required_field.count(titles[k]) && (cell->str == NULL || cell->str.empty())) {
                    msg = "第" + std::to_string(j + 1) + "行，学生信息填写不完整。";
                    err_msg.emplace_back(msg);
                    break;
                }
                if (cell->str != NULL) {
                    datas.emplace_back(cell->str);
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
