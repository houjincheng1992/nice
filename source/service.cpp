#include "service.h"

#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/join.hpp>

#include <butil/time.h>
#include "xls.h"

#include "utils/logger.h"
#include "utils/numutils.h"

namespace nicer {

static const std::set<std::string> required_fields = {
    "年级编号", "班级编号", "班级名称", "学籍号", "民族代码", "姓名", "性别", "出生日期", "家庭住址",
};

static const std::set<std::string> optional_fields = {
    "身高", "体重", "肺活量", "50米跑", "坐位体前屈", "一分钟跳绳", "一分钟仰卧起坐", "50米×8往返跑",
    "引体向上", "400米跑", "立定跳远", "1000米跑", "800米跑", "左眼裸眼视力", "右眼裸眼视力",
    "左眼串镜", "右眼串镜", "左眼屈光不正", "右眼屈光不正",
}

bool row_validate(
        std::map<std::string, int32_t>& title_index,
        std::vector<std::string>& row_data,
        int32_t row_num,
        std::vector<std::string>& err_msg) {
    std::string msg;
    for (auto &field_name : optional_fields) {
        switch (field_name) {
            case "身高":
                float height = NumUtils::stof(row_data[title_index[field_name]]);
                if(height < 80 || height > 250){
                    msg = "第" + std::to_string(row_data + 1) + "行, 身高：数据有误，数据格式错误或者超出数据导入范围（80—250厘米）。";
                    err_msg.emplace_back(msg);
                }
                break;
            case "体重":
                float weight = NumUtils::stof(row_data[title_index[field_name]]);
                if(weight < 14 || weight > 200){
                    msg = "第" + std::to_string(row_data + 1) + "行, 体重：数据有误，数据格式错误或者超出数据导入范围（14—200公斤）。";
                    err_msg.emplace_back(msg);
                }
                break;
            case "肺活量":
                int32_t lung_capacity = NumUtils::stoi(row_data[title_index[field_name]]);
                if (lung_capacity < 500 || lung_capacity > 9999) {
                    msg = "第" + std::to_string(row_data + 1) + "行, 肺活量：数据有误，数据格式错误或者超出数据导入范围（500－9999毫升）。";
                    err_msg.emplace_back(msg);
                }
                break;
        }
    }
}

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

        INFLOG << "rows: " << rows_num << ", cols: " << cols_num;

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

            bool is_row_empty = true;
            bool is_incomplete = false;

            std::vector<std::string> row_data;
            for (int32_t k = 0; k < cols_num; k++) {
                xls::xlsCell *cell = &row->cells.cell[k];
                if (required_fields.count(titles[k]) && cell->str == NULL) {
                    is_incomplete = true;
                    break;
                }
                is_row_empty = false;
                std::string col_data = cell->str == NULL ? "" : cell->str;
                row_data.emplace_back(col_data);
                row_validate(title_index, row_data, j, err_msg);
            }

            if (is_row_empty) {
                break;
            } else if (is_incomplete) {
                msg = "第" + std::to_string(j + 1) + "行，学生信息填写不完整。";
                err_msg.emplace_back(msg);
                continue;
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
