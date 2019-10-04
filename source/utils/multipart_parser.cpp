#include "multipart_parser.h"

#include <ctype.h>

namespace nicer {
namespace utils {

#define CR 13
#define LF 10

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

void MultipartParser::set_boundary(std::string& content_type) {
    std::string boundary = parse_boundary_by_contenttype(content_type);
    _boundary = boundary;
    return;
}

int32_t MultipartParser::parser(std::string& str) {
    int32_t index = 0;
    char c, cl;
    bool is_last = 0;
    while (index < str.size()) {
        c = str[index];
        is_last = (index == str.size() - 1);
        switch (_state) {
            case state::s_start:
                _cursor = 0;
                _state = state::s_start_boundary;

            case state::s_start_boundary:
                if (_cursor == _boundary.size()) {
                    if (c != CR) {
                        return index;
                    }
                    ++_cursor;
                    break;
                } else if (_cursor == _boundary.size() + 1) {
                    if (c != LF) {
                        return index;
                    }
                    _cursor = 0;
                    _state = state::s_header_field_start;
                    break;
                }

                if (c != _boundary[_cursor]) {
                    return index;
                }

                ++_cursor;
                break;

            case state::s_header_field_start:
                _mark = index;
                _state = state::s_header_field;

            case state::s_header_field:
                if (c == CR) {
                    _state = state::s_headers_almost_done;
                    break;
                }

                if (c == ':') {
                    _last_field_name = std::string(&str[_mark], index - _mark + 1);
                    _state = state::s_header_value_start;
                    break;
                }

                cl = tolower(c);
                if (c != '-' && (cl < 'a' || cl > 'z')) {
                    return index;
                }

                if (is_last) {
                    _last_field_name = std::string(&str[_mark], index - _mark + 1);
                }

                break;

            case state::s_headers_almost_done:
                if (c != LF) {
                    return index;
                }
                _state = s::s_part_data_start;
                break;

            case state::s_header_value_start:
                if (c == ' ') {
                    break;
                }
                _mark = index;
                _state = state::s_header_value;

            case state::s_header_value:
                if (c == CR) {
                    _last_field_value = std::string(&str[_mark], index - _mark + 1);
                    _state = state::s_header_value_almost_done;
                    break;
                }
                if (is_last) {
                    _last_field_value = std::string(&str[_mark], index - _mark + 1);
                }
                break;

            case state::s_header_value_almost_done:
                if (c != LF) {
                    return index;
                }
                _state = state::s_header_field_start;
                break;

            case state::s_part_data_start:
                _mark = index;
                _state = state::s_part_data;

            case state::s_part_data:
                if (c == CR) {
                    _mark = index;
                    _state = state::s_part_data_almost_boundary;
                    _look_behind.append(c);
                    break;
                }

                if (is_last) {
                    return index;
                }
                break;

            case state::s_part_data_almost_boundary:
                if (c == LF) {
                    _state = state::s_part_data_boundary;
                    _look_behind.append(c);
                    break;
                }
                _state = state::s_part_data;
                _mark = index--;
                break;

            case state::s_part_data_boundary:
                
        }
    }
    return str.size();
}

}
}