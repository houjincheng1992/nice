#pragma once

#include <string>

#include "utils/logger.h"

namespace nicer {
namespace xlsx {
class Xlsx {
public:
	Xlsx();
	~Xlsx();
	void set_decompressed_data(std::string* data) {
		decompressed_data = data;
		return;
	}

	std::string dump() {
		INFLOG << "dump " << *decompressed_data;
		return "";
	}
private:
	std::string*  decompressed_data = nullptr;
};
Xlsx xlsx_open_buffer(std::string& data);

}	// namespace xlsxutils
}	// namespace nicer