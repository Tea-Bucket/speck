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
  std::unordered_map<std::string, std::pair<uint64_t, uint64_t>>
      file_info; // position in speckage, file size
  char *data;
  uint64_t data_size = 0;
  uint64_t first_empty_location = 0;
  uint64_t min_memory_on_expand = 0; // minimum memory to add to data, when expanding
};

bool add_file_to_speckage(speckage &speckage, const std::string& filepath);

bool save_speckage_to_file(const speckage &toSave, const std::string& filepath);

template <typename data_type>
data_type get_data_from_stream(std::ifstream &file) {
  char out[sizeof(data_type)];

  for (unsigned long i = 0; i < sizeof(data_type); ++i) {
    out[i] = file.get();
  }
  data_type *return_value = reinterpret_cast<uint64_t *>(out);
  return *return_value;
};

speckage read_speckage_from_file(std::string filepath);;

void unload_speckage(speckage &speckage_to_unload);;

///
/// @param speckage speckage to read from
/// @param filepath relative path inside speckage
/// @param size is output
/// @return pointer to beginning of output
char *read_file_from_speckage(const speckage &speckage, std::string filepath,
                          uint64_t &size);;
} // namespace speck
