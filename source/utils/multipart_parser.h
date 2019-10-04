#pragma once

#include <map>
#include <string>

namespace nicer {
namespace utils {

enum class state {
    s_uninitialized = 1,
    s_start,
    s_start_boundary,
    s_header_field_start,
    s_header_field,
    s_headers_almost_done,
    s_header_value_start,
    s_header_value,
    s_header_value_almost_done,
    s_part_data_start,
    s_part_data,
    s_part_data_almost_boundary,
    s_part_data_boundary,
    s_part_data_almost_end,
    s_part_data_end,
    s_part_data_final_hyphen,
    s_end
};

class MultipartParser {
public:
    MultipartParser() {};
    ~MultipartParser() {};

    // 获取boundary 与 设置boundary
    void set_boundary(std::string& boundary);

    // return non 0 for error, 0 for succ
    // 解析multipart/form-data
    int32_t parser(std::string& str);
private:
    std::string parse_boundary_by_contenttype(std::string& content_type);

    struct Multipart {
        std::string name;
        const char* start;
        size_t len;
        std::string filename;
    };
    std::map<std::string, Multipart> _datas;
    std::string _boundary;

    state _state;
    int32_t _cursor;    // 当前偏移量
    int32_t _mark;
    std::string _last_field_name;
    std::string _last_field_value;
    std::string _look_behind;
};

}   // namespace utils
}   // namespace nicer