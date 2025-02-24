#include <cassert>
#include <cstdint>
#include <fstream>

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#define SPECK_STRING "SPECK"
#define SPECK_VERSION "1.0"

#define SPECK_STRING_LENGTH 6
#define SPECK_VERSION_LENGTH 4

namespace speck {
struct speckage {
    std::string                                                    name;
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info; // position in speckage, file size
    char*                                                          data;
    uint64_t                                                       data_size            = 0;
    uint64_t                                                       first_empty_location = 0;
    uint64_t                                                       min_memory_on_expand = 0; // minimum memory to add to data, when expanding
};

struct appendedSpeckageInfos {
    std::unordered_map<std::string, size_t>      speckage_offset_map;
    std::unordered_map<std::string, std::string> filename_speckage_map;
};

appendedSpeckageInfos discover_appended_speckages(const std::string& filepath);
appendedSpeckageInfos discover_appended_speckages(std::istream& stream);

speckage              read_appended_speckage_from_file(const std::string& speckage_name, const appendedSpeckageInfos& appended_speckage_infos, const std::string& filepath);
std::vector<speckage> read_appended_speckages_from_file(const std::vector<std::string>& speckage_names, const appendedSpeckageInfos& appended_speckage_infos, const std::string& filepath);

speckage              read_appended_speckage_from_stream(const std::string& speckage_name, const appendedSpeckageInfos& appended_speckage_infos, std::istream& stream);
std::vector<speckage> read_appended_speckages_from_stream(const std::vector<std::string>& speckage_names, const appendedSpeckageInfos& appended_speckage_infos, std::istream& stream);

bool                  append_speckage_to_file(const speckage& speckage, const std::string& filepath);
bool                  append_speckages_to_file(const std::vector<speckage>& speckages, const std::string& filepath);

bool                  append_speckage_to_stream(const speckage& speckage, std::ostream& stream);
bool                  append_speckages_to_stream(const std::vector<speckage>& speckage, std::ostream& stream);

bool                  add_file_to_speckage(speckage& speckage, const std::string& filepath);

bool                  save_speckage_to_file(const speckage& toSave, const std::string& filepath);

speckage              read_speckage_from_file(std::string filepath);

void                  unload_speckage(speckage& speckage_to_unload);

///
/// @param speckage speckage to read from
/// @param filepath relative path inside speckage
/// @param size is output
/// @return pointer to beginning of output
char*                 read_file_from_speckage(const speckage& speckage, std::string filepath, uint64_t& size);

template <typename data_type>
data_type get_data_from_stream(std::ifstream& file)
{
    char out[sizeof(data_type)];

    for (unsigned long i = 0; i < sizeof(data_type); ++i)
    {
        out[i] = file.get();
    }
    data_type* return_value = reinterpret_cast<uint64_t*>(out);
    return *return_value;
};

} // namespace speck
