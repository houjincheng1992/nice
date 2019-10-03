#pragma once

#include <map>
#include <string>
#include <utility>

#include "utls/multipart_parser.h"

namespace nicer {
namespace svc {

/*
 *  multipart回调函数类
 */
class MultipartCallBack {
public:
    MultipartCallBack() {};
    ~MultipartCallBack() {};

    int32_t on_boundary_begin(utils::multipart_parser* parser) {
        _last_field_name = "";
        _last_header_name = "";
        _last_filename = "";
        return 0;
    }

    int32_t on_headers_complete(utils::multipart_parser* parser) {
        return 0;
    }

    int32_t on_body_parts_complete(utils::multipart_parser* parser) {
        return 0;
    }

    int32_t on_header_field(utils::multipart_parser* parser, const char *at, size_t length) {
        if (parser == nullptr || at == nullptr) {
            return -1;
        }
        std::string header_field = std::string(at, length)
        if (header_field == "Content-Type" || header_field == "Content-Disposition") {
            _last_field_name = header_field;
        } else {
            _last_field_name = "";
            return -1;
        }
        return 0;
    }

    int32_t on_header_value(utils::multipart_parser* parser, const char *at, size_t length) {
        if (parser == nullptr || at == nullptr) {
            return -1;
        }

        size_t size;
        if (_last_field_name == "Content-Disposition") {
            const char* header_name = utils::multipart_get_name(at, length, &size);
            if (header_name == nullptr) {
                return -1;
            }
            _last_header_name = std::string(header_name, size);

            const char* filename = utils::multipart_get_filename(at, length, &size);
            if (filename == nullptr) {
                return -1;
            }
            _last_filename = std::string(filename, size);
        }
        return 0;
    }

    int32_t on_body(utils::multipart_parser* parser, const char *at, size_t length) {
        if (parser == nullptr || at == nullptr) {
            return -1;
        }
        _datas.emplace(_last_header_name, std::make_pair(at, length));
        if (!_last_filename.empty()) {
            _field_filename.emplace(_last_header_name, _last_filename);
        }
        return 0;
    }

    const char* get_content(std::string& field_name, size_t& len) {
        if (_datas.count(field_name)) {
            len = _datas[field_name].second;
            return _datas[field_name].first;
        }
        return nullptr;
    }

    std::string get_filename(std::string& field_name) {
        return _field_filename.count(field_name) ? _field_filename[field_name] : "";
    }
private:
    // {field_name, {field_value_pointer, field_value_len}}
    std::map<std::string, std::pair<const char*, size_t len>> _datas;
    std::map<std::string, std::string> _field_filename;
    std::string _last_field_name;
    std::string _last_header_name;
    std::string _last_filename;
};

}   // namespace svc
}   // namespace nicer