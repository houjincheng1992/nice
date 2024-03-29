#pragma once

#include <string>

namespace nicer {
namespace utils {
class Uri {
public:
    static std::string decode(const std::string& str) {
        std::string url_decode_str;
        url_decode_str.reserve(str.size());

        for (uint32_t u = 0; u < str.size(); ++u) {
            const char c = str.at(u);
            if (c == '%') {
                if (u + 2 >= str.size()) {
                    /* 容错情况,理论上这种情况不会出现 */
                    break;
                }
                uint8_t high = decode_char(str[++u]);
                uint8_t low = decode_char(str[++u]);
                url_decode_str.push_back((high << 4) | low);
                continue;
            }

            if (c == '+') {
                url_decode_str.push_back(' ');
                continue;
            }

            url_decode_str.push_back(c);
        }

        return url_decode_str;
    }
private:
    inline static uint8_t decode_char(const char& c) {
        if ('0' <= c && c <= '9') {
            return c - '0';
        }

        if ('A' <= c && c <= 'F') {
            return c - 'A' + 10;
        }

        if ('a' <= c && c <= 'f') {
            return c - 'a' + 10;
        }

        /* 容错情况,不会出现 */
        return c;
    }
};
}
}