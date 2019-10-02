#pragma once

#include <string>

namespace nicer {
namespace utils {

class NumUtils {
public:
	static float stof(std::string& str) noexcept {
		try {
			float value = std::stof(str);
			return value;
		} catch (...) {
			return 0.0;
		}
	}

	static int32_t stoi(std::string& str) noexcept {
		try {
			int32_t value = std::stoi(str);
			return value;
		} catch (...) {
			return 0;
		}
	}
};

}	// namespace utils
}	// namespace nicer