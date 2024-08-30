
#include <cstdint>
#include <fstream>
#include <map>

#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace speck{
	struct speckage{
		std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info;
		void* data;
		uint64_t data_size;
	};


	bool addFileToPackage(speckage& speckage, const char* filepath)
	{
		std::ifstream file (filepath, std::ios::in|std::ios::binary|std::ios::ate);
		if(!file){
			//file failed to open
			return false;
		}
		auto file_size = file.tellg();
		file.seekg(0, std::ios::beg);

		void* next_data = malloc(speckage.data_size + file_size);

		std::memcpy(next_data, speckage.data, speckage.data_size);

		file.read(reinterpret_cast<char*>(next_data) + speckage.data_size, file_size);

		if(file.fail()){
			//io error
			free(next_data);
			return false;
		}

		free(speckage.data);
		speckage.data_size += file_size;
		speckage.data = next_data;
		return true;
	}
	bool savePackageToFile(const speckage& toSave, char* filepath);
	speckage readPackageFromFile(std::string filepath)
	{
		std::ifstream file (filepath);
		uint64_t header_size;
		file >> header_size;
		while (true)
		{

		}
		return speckage();
	};

	///
	/// @param speckage speckage to read from
	/// @param filepath relative path inside speckage
	/// @param size is output
	/// @return pointer to beginning of output
	char* readFileFromPackage(const speckage& speckage, std::string filepath, uint32_t& size)
	{

	};
}
