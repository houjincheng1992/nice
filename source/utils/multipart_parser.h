#pragma once

#include <map>
#include <string>

namespace nicer {
namespace utils {

class MultipartParser {
public:
    MultipartParser() {};
    ~MultipartParser() {};

    // 获取boundary 与 设置boundary
    std::string parse_boundary_by_contenttype(std::string& content_type);
    void set_boundary(std::string& boundary);

    // return non 0 for error, 0 for succ
    // 解析multipart/form-data
    int32_t parser(std::string& str);
private:
    struct Multipart {
        std::string name;
        const char* start;
        size_t len;
        std::string filename;
    };
    std::map<std::string, Multipart> _datas;
    std::string _boundary;
};

}   // namespace utils
}   // namespace nicer