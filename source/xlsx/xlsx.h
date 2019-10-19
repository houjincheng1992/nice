#pragma once

#include <string>

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
		#include <iostream>
		std::cout << "dump " << *decompressed_data << std::endl;
	}
private:
	std::string*  decompressed_data = nullptr;
};
Xlsx xlsx_open_buffer(std::string& data);
}	// namespace xlsxutils
}	// namespace nicer