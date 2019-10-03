#include "multipart_parser.h"

namespace nicer {
namespace utils {

std::string MultipartParser::parse_boundary_by_contenttype(std::string& content_type) {
    size_t size = content_type.find("boundary=");
    if (size == std::string::npos) {
        return "";
    }
    size_t pos = content_type.find(";", size);
    if (pos != std::string::npos) {
        return content_type.substr(size + 9, pos - size - 9);
    }
    return content_type.substr(size + 9);
}

}
}