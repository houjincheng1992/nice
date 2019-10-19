#pragma once

#include <map>
#include <string>
#include <vector>

namespace nicer {
namespace svc {

enum class FileType {
    XLS,
    XLSX,
    NOTVALID,
};  // 文件类型

static FileType get_file_type(const std::string& filename) {
    FileType file_type = FileType::NOTVALID;
    if (filename.find(".xlsx") != std::string::npos) {
        file_type = FileType::XLSX;
    } else if (filename.find(".xls") != std::string::npos) {
        file_type = FileType::XLS;
    }
    return file_type;
}

static const std::set<std::string> required_fields = {
    "年级编号", "班级编号", "班级名称", "学籍号", "民族代码", "姓名", "性别", "出生日期", "家庭住址",
};

static const std::vector<std::string> optional_fields = {
    "身高", "体重", "肺活量", "50米跑", "坐位体前屈", "一分钟跳绳", "一分钟仰卧起坐", "50米×8往返跑",
    "引体向上", "400米跑", "立定跳远", "1000米跑", "800米跑", "左眼裸眼视力", "右眼裸眼视力",
    "左眼串镜", "右眼串镜", "左眼屈光不正", "右眼屈光不正",
};

// class CheckService {
// public:
//     CheckService();
//     ~CheckService();

//     bool validate_excel(const std::string& excel_data, const std::string& filename, std::vector<std::string>& err_msg);
// private:
//     bool row_validate(
//             std::map<std::string, int32_t>& title_index,
//             std::vector<std::string>& row_data,
//             int32_t row_num,
//             std::vector<std::string>& err_msg);

//     std::string msg_format(int32_t row_num, const std::string& name, const std::string& standard) {
//         return "第" + std::to_string(row_num) + "行, " + name + "：数据有误，数据格式错误或者超出数据导入范围（"
//                     + standard + "）。";
//     }
// };
}   // namespace svc
}   // namespace nicer