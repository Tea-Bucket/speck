#include <cassert>
#include <cstdint>
#include <fstream>

#include <cstring>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#define SPECK_STRING "SPECK"
#define SPECK_VERSION "1.3"

#define SPECK_STRING_LENGTH 6
#define SPECK_VERSION_LENGTH 4

#define SPECK_FOOTER_STRING "SPECK_APPEND"

#define SPECK_FOOTER_STRING_LENGTH 13

namespace speck {
struct data_range {
    uint64_t offset;
    uint64_t length;
};
struct speckage_read_info {
    std::string                                 name;           // maximum of 255 non-wide chars
    std::string                                 speckage_path;  //actual file on disk the speckage is located in
    data_range                                  speckage_range; // space the speckage occupies inside the file that contains it
    uint64_t                                    header_size;    //size of the header
    std::unordered_map<std::string, data_range> file_info;      // position in speckage, file size
};

/// parses all speckages that are appended to a file. also reads standalone speckage files
/// @param filepath file to read
/// @return speckages
std::vector<speckage_read_info> read_speckages_from_file(const std::filesystem::path& filepath);

///
/// @param speckage speckage to read from
/// @param filepath relative path inside speckage
/// @param size is output
/// @return pointer to beginning of output. needs to be freed with speck_free() when it is no longer needed
char*                           read_file_from_speckage(const speckage_read_info& speckage, const std::string& filepath, uint64_t& size);

void                            speck_free(char* allocation);

} // namespace speck
namespace speck_write {

struct speckage_for_write {
    std::string                                                    name;      // maximum of 255 non-wide chars
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info; // position in speckage, file size
    char*                                                          data;
    uint64_t                                                       data_size            = 0;
    uint64_t                                                       first_empty_location = 0;
    uint64_t                                                       min_memory_on_expand = 0; // minimum memory to add to data, when expanding
};
bool append_speckages_to_file(const std::vector<speck::speckage_read_info>& speckages, const std::filesystem::path& filepath);

bool save_speckage_to_file(const speckage_for_write& toSave, const std::string& filepath);

bool add_file_to_speckage(speckage_for_write& speckage, const std::string& filepath);

void unload_speckage(speckage_for_write& speckage_to_unload);
} // namespace speck_write