#include "service.h"

#include <sstream>
#include <limits>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

#include <butil/time.h>
#include "xls.h"

#include "svc/callback.h"
#include "utils/logger.h"
#include "utils/multipart_parser.h"
#include "utils/numutils.h"

namespace nicer {

static const std::set<std::string> required_fields = {
    "年级编号", "班级编号", "班级名称", "学籍号", "民族代码", "姓名", "性别", "出生日期", "家庭住址",
};

static const std::vector<std::string> optional_fields = {
    "身高", "体重", "肺活量", "50米跑", "坐位体前屈", "一分钟跳绳", "一分钟仰卧起坐", "50米×8往返跑",
    "引体向上", "400米跑", "立定跳远", "1000米跑", "800米跑", "左眼裸眼视力", "右眼裸眼视力",
    "左眼串镜", "右眼串镜", "左眼屈光不正", "右眼屈光不正",
};

std::string msg_format(int32_t row_num, const std::string& name, const std::string& standard) {
    return "第" + std::to_string(row_num) + "行, " + name + "：数据有误，数据格式错误或者超出数据导入范围（" + standard + "）。";
}

bool row_validate(
        std::map<std::string, int32_t>& title_index,
        std::vector<std::string>& row_data,
        int32_t row_num,
        std::vector<std::string>& err_msg) {
    std::string msg;

    bool is_high_grade = utils::NumUtils::stoi(row_data[title_index["年级编号"]]) > 20;
    bool is_male = row_data[title_index["性别"]] == "1";
    for (auto &field_name : optional_fields) {
        if (field_name == "身高") {
            float height = utils::NumUtils::stof(row_data[title_index[field_name]]);
            if(height < 80 || height > 250){
                msg = msg_format(row_num + 1, field_name, "80—250厘米");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "体重") {
            float weight = utils::NumUtils::stof(row_data[title_index[field_name]]);
            if(weight < 14 || weight > 200){
                msg = msg_format(row_num + 1, field_name, "14—200公斤");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "肺活量") {
            int32_t lung_capacity = utils::NumUtils::stoi(row_data[title_index[field_name]]);
            if (lung_capacity < 500 || lung_capacity > 9999) {
                msg = msg_format(row_num + 1, field_name, "500－9999毫升");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "50米跑") {
            float run_50m = utils::NumUtils::stof(row_data[title_index[field_name]]);
            if (run_50m < 5 || run_50m > 20) {
                msg = msg_format(row_num + 1, field_name, "5-20秒");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "坐位体前屈") {
            float sitting_body_flexion = utils::NumUtils::stof(row_data[title_index[field_name]]);
            if (sitting_body_flexion < -30 || sitting_body_flexion > 40) {
                msg = msg_format(row_num + 1, field_name, "－30—40厘米");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "一分钟跳绳") {
            float skip_one_minute = utils::NumUtils::stoi(row_data[title_index[field_name]]);
            if (skip_one_minute < 0 || skip_one_minute > 300) {
                msg = msg_format(row_num + 1, field_name, "0—300次/分钟");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "一分钟仰卧起坐") {
            std::string sit_up_one_minute_str = row_data[title_index[field_name]];
            if (is_high_grade && is_male && !sit_up_one_minute_str.empty()) {
                msg = "第" + std::to_string(row_num + 1) + "行," + field_name + "：男生没有此测试项目，不能有值。";
                err_msg.emplace_back(msg);
                return false;
            } else if (is_high_grade && is_male) {
                continue;
            }
            int32_t sit_up_one_minute = utils::NumUtils::stoi(sit_up_one_minute_str);
            if (sit_up_one_minute < 0 || sit_up_one_minute > 99) {
                msg = msg_format(row_num + 1, field_name, "0－99次/分钟");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "50米×8往返跑" || field_name == "400米跑") {
            std::string round_trip_50_8 = row_data[title_index[field_name]];
            if (round_trip_50_8.empty()) {
                msg = msg_format(row_num + 1, field_name, "0′45″—4′00″分·秒");
                err_msg.emplace_back(msg);
                return false;
            }
            std::set<std::string> seperate = {"\"", "″", "”"};
            if (seperate.count(&round_trip_50_8[-1])) {
                round_trip_50_8 = round_trip_50_8.substr(0, round_trip_50_8.size() - 1);
            }

            std::vector<std::string> split_vec;
            boost::split(split_vec, round_trip_50_8, boost::is_any_of("′’'"));
            if (split_vec.size() == 2) {
                if (split_vec[1].size() != 2) {
                    msg = msg_format(row_num + 1, field_name, "0′45″—4′00″分·秒");
                    err_msg.emplace_back(msg);
                    return false;
                }
                int32_t minute = utils::NumUtils::stoi(split_vec[0]);
                int32_t second = utils::NumUtils::stoi(split_vec[1]);

                if (minute < 0 || minute > 4 || minute == 4 && second != 0 || second < 0 || second > 59) {
                    msg = msg_format(row_num + 1, field_name, "0′45″—4′00″分·秒");
                    err_msg.emplace_back(msg);
                    return false;
                }
            }
            continue;
        } else if (field_name == "引体向上") {
            std::string pull_up_str = row_data[title_index[field_name]];
            if (is_high_grade && !is_male && !pull_up_str.empty()) {
                msg = "第" + std::to_string(row_num + 1) + "行," + field_name + "：女生没有此测试项目，不能有值。";
                err_msg.emplace_back(msg);
                return false;
            } else if (is_high_grade && !is_male) {
                continue;
            }
            int32_t pull_up = utils::NumUtils::stoi(pull_up_str);
            if (pull_up < 0 || pull_up > 99) {
                msg = msg_format(row_num + 1, field_name, "0—99次");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "立定跳远") {
            int32_t stand_jump = utils::NumUtils::stoi(row_data[title_index[field_name]]);
            if (stand_jump < 50 || stand_jump > 400) {
                msg = msg_format(row_num + 1, field_name, "50—400厘米");
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "1000米跑" || field_name == "800米跑") {
            std::string run_1km = row_data[title_index[field_name]];
            if (field_name == "1000米跑" && is_high_grade && !is_male && !run_1km.empty()) {
                msg = "第" + std::to_string(row_num + 1) + "行," + field_name + "：女生没有此测试项目，不能有值。";
                err_msg.emplace_back(msg);
                return false;
            } else if (field_name == "800米跑" && is_high_grade && is_male && !run_1km.empty()) {
                msg = "第" + std::to_string(row_num + 1) + "行," + field_name + "：男生没有此测试项目，不能有值。";
                err_msg.emplace_back(msg);
                return false;
            } else if (field_name == "1000米跑" && is_high_grade && !is_male || field_name == "800米跑" && is_high_grade && is_male) {
                continue;
            }
            if (run_1km.empty()) {
                msg = msg_format(row_num + 1, field_name, "2′00″－9′00″分·秒");
                err_msg.emplace_back(msg);
                return false;
            }
            std::set<std::string> seperate = {"\"", "″", "”"};
            if (seperate.count(&run_1km[-1])) {
                run_1km = run_1km.substr(0, run_1km.size() - 1);
            }

            std::vector<std::string> split_vec;
            boost::algorithm::split(split_vec, run_1km, boost::is_any_of("′’'"));
            if (split_vec.size() == 2) {
                if (split_vec[1].size() != 2) {
                    msg = msg_format(row_num + 1, field_name, "2′00″－9′00″分·秒");
                    err_msg.emplace_back(msg);
                    return false;
                }
                int32_t minute = utils::NumUtils::stoi(split_vec[0]);
                int32_t second = utils::NumUtils::stoi(split_vec[1]);

                if (minute < 2 || minute > 9 || minute == 9 && second != 0 || second < 0 || second > 59) {
                    msg = msg_format(row_num + 1, field_name, "2′00″－9′00″分·秒");
                    err_msg.emplace_back(msg);
                    return false;
                }
            }
            continue;
        } else if (field_name == "左眼裸眼视力" || field_name == "右眼裸眼视力") {
            float nake_vision = utils::NumUtils::stof(row_data[title_index[field_name]]);
            if (nake_vision != 0 && (nake_vision < 3 || nake_vision > 5.3)) {
                msg = msg_format(row_num + 1, field_name, "0或者3~5.3");
                err_msg.emplace_back(msg);
                return false;
            }

            std::string tandem_mirror_name = field_name == "左眼裸眼视力" ? "左眼串镜" : "右眼串镜";
            float tandem_mirror_value = utils::NumUtils::stof(row_data[title_index[tandem_mirror_name]]);
            if ((nake_vision > 5.0
                    || fabs(nake_vision - 5.0) <= std::numeric_limits<float>::epsilon())
                            && fabs(tandem_mirror_value - 0.0) > std::numeric_limits<float>::epsilon()) {
                msg = "第" + std::to_string(row_num + 1) + "行, " + tandem_mirror_name + "：裸眼视力大于等于5.0，无需使用串镜检查，数据只能为0。";
                err_msg.emplace_back(msg);
                return false;
            }

            if (nake_vision < 5.0
                    && (fabs(tandem_mirror_value - 1.0) > std::numeric_limits<float>::epsilon())
                    && (fabs(tandem_mirror_value + 1.0) > std::numeric_limits<float>::epsilon())
                    && (fabs(tandem_mirror_value - 2.0) > std::numeric_limits<float>::epsilon())
                    && (fabs(tandem_mirror_value - 9.0) > std::numeric_limits<float>::epsilon())) {
                msg = "第"+ std::to_string(row_num + 1)+"行, " + tandem_mirror_name + "：裸眼视力小于5.0，数据只能为-1,1,2,9。";
                err_msg.emplace_back(msg);
                return false;
            }
            continue;
        } else if (field_name == "左眼屈光不正" || field_name == "右眼屈光不正") {
            int32_t ametropia = utils::NumUtils::stoi(row_data[title_index[field_name]]);
            if (ametropia < 0 || ametropia > 3 && ametropia != 9) {
                msg = "第" + std::to_string(row_num + 1) + "行, " + field_name + "：数据有误，数据只能为0,1,2,3,9。";
                err_msg.emplace_back(msg);
                return false;
            }
        }
    }
    return true;
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
    std::string content_type = ctrl->http_request().content_type();

    size_t boundary_len;
    const char* boundary = utils::get_boundary(content_type.c_str(), content_type.size(), &boundary_len);
    utils::multipart_parser parser;
    parser.boundary = boundary;
    parser.boundary_len = boundary_len;
    utils::multipart_parser_init(&parser);

    svc::MultipartCallBack multipart_callback;
    utils::multipart_parser_settings parser_settings;
    utils::multipart_parser_settings_init(&parser_settings);

    parser_settings.on_boundary_begin = multipart_callback.on_boundary_begin;
    parser_settings.on_headers_complete = multipart_callback.on_headers_complete;
    parser_settings.on_body_parts_complete = multipart_callback.on_body_parts_complete;
    parser_settings.on_header_field = multipart_callback.on_header_field;
    parser_settings.on_header_value = multipart_callback.on_header_value;
    parser_settings.on_body = multipart_callback.on_body;
    utils::multipart_parser_execute(&parser, &parser_settings, str.c_str(), str.size());

    size_t len;
    std::string file_field_name = "update_file";
    const char* file_content = multipart_callback.get_content(file_field_name, len);
    std::string filename = multipart_callback.get_filename(file_field_name);

    xls::xls_error_t error = xls::LIBXLS_OK;
    xls::xlsWorkBook *wb = xls::xls_open_buffer((const unsigned char*)file_content, len, "UTF-8", &error);
    if (wb == NULL) {
        ERRLOG << "error reading file: " << xls_getError(error);
        std::string response_str = "{\"msg\":\"done\"}";
        ctrl->response_attachment().append(response_str);
        return;
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
            }

            if (is_row_empty) {
                break;
            } else if (is_incomplete) {
                msg = "第" + std::to_string(j + 1) + "行，学生信息填写不完整。";
                err_msg.emplace_back(msg);
                continue;
            }
            row_validate(title_index, row_data, j, err_msg);
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
