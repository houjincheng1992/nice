#include "xlsx.h"

#include <gzip/decompress.hpp>

namespace nicer {
namespace xlsx {

Xlsx::Xlsx() {}

Xlsx::~Xlsx() {
	if (decompressed_data != nullptr) {
		delete decompressed_data;
	}
}

Xlsx xlsx_open_buffer(std::string& data) {
	gzip::Decompressor decomp;
	std::string* decompressed_data = new std::string();
	try {
		INFLOG << "data: " << data;
		decomp.decompress(*decompressed_data, data.c_str(), data.size());
	} catch (const std::exception& e) {
		INFLOG << "decompress exception: " << e.what();
	}
	
	Xlsx xlsx;
	xlsx.set_decompressed_data(decompressed_data);
	return xlsx;
}

}	// namespace xlsx
}	// namespace nicer