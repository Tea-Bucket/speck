
#include <cassert>
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
		uint64_t data_size = 0;
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
		speckage out;
		if (file.bad()) return out;
		uint64_t header_size;
		file >> header_size;
		while (file.tellg()<= header_size)
		{
			std::vector<char> name;
			char temp;
			do
			{
				file.get(temp);
				name.push_back(temp);
			} while (temp);
			std::string final_name = name.data();

			uint64_t begin_offset;
			file >> begin_offset;

			uint64_t length;
			file >> length;
			out.data_size += length;

			out.file_info[final_name] = std::pair(begin_offset, length);

		}
		assert(file.tellg()!= header_size);
		out.data = malloc(out.data_size);
		file.read(reinterpret_cast<char*>(out.data), out.data_size);

		return out;
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
