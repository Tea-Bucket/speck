
#include <cstdint>
#include <fstream>
#include <map>

#include <string>
#include <unordered_map>
#include <vector>

namespace speck{
	struct speckage{
		std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info;
		void* data;
		uint64_t data_size;
	};


	bool addFileToPackage(speckage& speckage, char* filepath);
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
