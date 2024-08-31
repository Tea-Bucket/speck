
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
	template<typename data_type>
	data_type get_data_from_stream(std::ifstream& file)
	{
		char out[sizeof(data_type)];

		for (int i = 0; i < sizeof(data_type); ++i)
		{
			file.get(out[i]);
		}
		data_type* return_value = reinterpret_cast<uint64_t*>(out);
		return *return_value;
	};
	speckage readPackageFromFile(std::string filepath)
	{
		//std::ifstream file (filepath);
		auto file = std::ifstream(filepath);
		speckage out;
		if (file.bad()) return out;
		auto header_size = get_data_from_stream<uint64_t>(file);
		//char test;
		//file.get(test);
		//file >> header_size;
		while ((uint64_t) file.tellg() < header_size)
		{
			std::vector<char> name;
			char temp;
			do
			{
				file.get(temp);
				name.push_back(temp);
			} while (temp);
			std::string final_name = name.data();


			auto begin_offset = get_data_from_stream<uint64_t>(file);


			auto length = get_data_from_stream<uint64_t>(file);
			out.data_size += length;

			out.file_info[final_name] = std::pair(begin_offset, length);

		}
		assert((uint64_t) file.tellg()== header_size);
		out.data = malloc(out.data_size);
		file.read(reinterpret_cast<char*>(out.data), out.data_size);

		return out;
	};

	///
	/// @param speckage speckage to read from
	/// @param filepath relative path inside speckage
	/// @param size is output
	/// @return pointer to beginning of output
	void* readFileFromPackage(const speckage& speckage, std::string filepath, uint64_t& size)
	{
		if(!(speckage.file_info.count(filepath))) return nullptr;
		std::pair<uint64_t, uint64_t> info = speckage.file_info.at(filepath);
		size = info.second;
		return speckage.data + info.first;
	};
}
