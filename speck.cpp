#include "include/speck.hpp"

namespace speck {

template <typename data_type>
data_type get_data_from_stream(std::istream& stream)
{
    char out[sizeof(data_type)];

    for (unsigned long i = 0; i < sizeof(data_type); ++i)
    {
        out[i] = stream.get();
    }
    data_type* return_value = reinterpret_cast<data_type*>(out);
    return *return_value;
};
bool read_header_from_stream(std::istream& stream, std::string& speckage_name, uint64_t& header_size, std::unordered_map<std::string, data_range>& file_info)
{
    size_t start_position = stream.tellg();

    // check header prefix
    char   name_string[SPECK_STRING_LENGTH];
    char   version_string[SPECK_VERSION_LENGTH];

    stream.read(name_string, SPECK_STRING_LENGTH);
    stream.read(version_string, SPECK_VERSION_LENGTH);

    if (std::strcmp(name_string, SPECK_STRING) != 0 || std::strcmp(version_string, SPECK_VERSION) != 0)
    {
        return false;
    }

    // read header
    header_size    = get_data_from_stream<uint64_t>(stream) + SPECK_STRING_LENGTH + SPECK_VERSION_LENGTH;
    auto name_size = get_data_from_stream<uint8_t>(stream);
    speckage_name.resize(name_size);
    stream.read(speckage_name.data(), name_size);
    while ((size_t)stream.tellg() < start_position + (size_t)header_size)
    {
        std::vector<char> name;
        char              temp;
        do
        {
            stream.get(temp);
            name.push_back(temp);
        }
        while (temp);
        std::string final_name   = name.data();

        auto        begin_offset = get_data_from_stream<uint64_t>(stream);

        auto        length       = get_data_from_stream<uint64_t>(stream);

        file_info[final_name]    = {begin_offset, length};
    }
    assert((size_t)stream.tellg() == start_position + header_size);
    return true;
}

speckage_read_info read_speckage_from_stream(std::istream& stream, const std::string& speckage_file_path, data_range speckage_data_range)
{
    stream.seekg(speckage_data_range.offset, std::ios_base::beg);
    speckage_read_info out;
    out.speckage_range = speckage_data_range;
    out.speckage_path  = speckage_file_path;
    if (!read_header_from_stream(stream, out.name, out.header_size, out.file_info))
    {
        return out;
    }

    return out;
}

struct footer_info {
    bool        ok;
    uint64_t    footer_length;
    std::string speckage_name;
    data_range  speckage_total;
};
/// evaluates the footer above the current stream position. changes stream position while doing so.
/// @param stream
/// @return info
footer_info read_footer(std::istream& stream)
{
    footer_info out;
    out.ok            = false;

    int64_t position   = stream.tellg();
    size_t footer_end = position;
    // check post-header postfix
    char   name_string[SPECK_FOOTER_STRING_LENGTH];
    char   version_string[SPECK_VERSION_LENGTH];

    position -= SPECK_FOOTER_STRING_LENGTH;
    if (position<0) return out;
    stream.seekg(position, std::ios_base::beg);
    stream.read(name_string, SPECK_FOOTER_STRING_LENGTH);

    position -= SPECK_VERSION_LENGTH;
    if (position<0) return out;
    stream.seekg(position, std::ios_base::beg);
    stream.read(version_string, SPECK_VERSION_LENGTH);

    // if post-header is invalid, all appended speckages have been found
    if (strcmp(name_string, SPECK_FOOTER_STRING) != 0 || strcmp(version_string, SPECK_VERSION) != 0)
    {
        return out;
    }

    // speckage name
    position -= sizeof(uint8_t);
    if (position<0) return out;
    stream.seekg(position, std::ios_base::beg);
    uint8_t name_length = 0;
    stream.read((char*)&name_length, sizeof(name_length));

    out.speckage_name.resize(name_length);
    position -= name_length;
    if (position<0) return out;
    stream.seekg(position, std::ios_base::beg);
    stream.read(out.speckage_name.data(), name_length);

    // read distance to header
    uint64_t size = 0;
    position -= sizeof(size);
    if (position<0) return out;
    stream.seekg(position, std::ios_base::beg);
    stream.read((char*)&size, sizeof(size));

    out.footer_length = footer_end - position;
    position -= size;
    out.speckage_total.offset = position;
    out.speckage_total.length = footer_end - position;
    out.ok                    = true;
    return out;
}

[[nodiscard]] std::vector<footer_info> find_appended_speckage_offsets(std::istream& stream)
{
    std::vector<footer_info> out;

    stream.seekg(0, std::ios_base::end);
    size_t position = stream.tellg(); //end of stream
    while (!stream.bad() && !stream.fail())
    {
        footer_info info = read_footer(stream);
        if (!info.ok)
            break;
        out.push_back(info);
        stream.seekg(info.speckage_total.offset, std::ios_base::beg);
    }
    return out;
}

std::vector<speckage_read_info> read_speckages_from_file(const std::filesystem::path& filepath)
{
    auto                            file = std::ifstream(filepath, std::ios::binary | std::ios::in);
    std::vector<speckage_read_info> out;
    if (file.bad())
        return out;
    auto footer_infos = find_appended_speckage_offsets(file);
    for (const auto& info : footer_infos)
    {
        out.push_back(read_speckage_from_stream(file, filepath, info.speckage_total));
    }
    return out;
}

char* read_file_from_speckage(const speckage_read_info& speckage, const std::string& filepath, uint64_t& size)
{
    if (!(speckage.file_info.count(filepath)))
        return nullptr;
    const auto& range = speckage.file_info.at(filepath);
    size              = range.length;

    auto file         = std::ifstream(speckage.speckage_path, std::ios::binary | std::ios::in);
    if (file.bad())
        return nullptr;
    file.seekg(speckage.speckage_range.offset + speckage.header_size + range.offset, std::ios_base::beg);

    char* out = (char*)malloc(range.length);
    file.read(out, range.length);
    return out;
}

void speck_free(char* allocation) { free(allocation); }
} // namespace speck
namespace speck_write {
void write_speckage_to_stream(const speckage_for_write& speckage, std::ostream& stream)
{
    size_t   position    = stream.tellp();
    uint64_t header_size = sizeof(uint64_t); // starting size for the size itself

    uint8_t  name_size   = (uint8_t)speckage.name.size();
    header_size += sizeof(name_size);
    header_size += name_size;

    for (const auto& info : speckage.file_info)
    {
        // two uint64_t for each file;
        header_size += 16;

        header_size += info.first.size() + 1;
    }

    stream.write(SPECK_STRING, SPECK_STRING_LENGTH);
    stream.write(SPECK_VERSION, SPECK_VERSION_LENGTH);
    stream.write(reinterpret_cast<char*>(&header_size), sizeof(header_size));
    stream.write((char*)&name_size, sizeof(name_size));
    stream.write(speckage.name.data(), name_size);
    for (const auto& info : speckage.file_info)
    {
        for (const char& character : info.first)
        {
            stream << character;
        }
        stream << '\0';
        // position
        stream.write(reinterpret_cast<const char*>(&info.second.first), 8);
        // size
        stream.write(reinterpret_cast<const char*>(&info.second.second), 8);
    }

    stream.write(reinterpret_cast<char*>(speckage.data), speckage.first_empty_location);

    uint64_t size = (size_t)stream.tellp() - position;

    // now write footer
    stream.write((char*)&size, sizeof(size));                      // size of speckage
    stream.write(speckage.name.data(), name_size);                 // name
    stream.write((char*)&name_size, sizeof(name_size));            // name size
    stream.write(SPECK_VERSION, SPECK_VERSION_LENGTH);             // version
    stream.write(SPECK_FOOTER_STRING, SPECK_FOOTER_STRING_LENGTH); // footer postfix
}

bool append_speckages_to_file(const std::vector<speck::speckage_read_info>& speckages, const std::filesystem::path& filepath)
{
    std::ofstream out_file(filepath, std::ios::out | std::ios::binary | std::ios::ate | std::ios::app);

    for (const auto& speckage : speckages)
    {
        auto in_file = std::ifstream(speckage.speckage_path, std::ios::binary | std::ios::in);
        if (in_file.bad())
            return false;
        in_file.seekg(speckage.speckage_range.offset, std::ios_base::beg);

        char* buffer = new char[speckage.speckage_range.length];
        in_file.read(buffer, speckage.speckage_range.length);
        out_file.write(buffer, speckage.speckage_range.length);
        delete[] buffer;
        if (in_file.fail() || out_file.fail())
            return false;
    }

    if (out_file.fail())
    {
        return false;
    }
    return true;
}

bool add_file_to_speckage(speckage_for_write& speckage, const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file)
    {
        // file failed to open
        return false;
    }
    auto file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (speckage.data_size - speckage.first_empty_location < (uint64_t)file_size)
    {
        uint64_t next_size = (uint64_t)file_size > speckage.min_memory_on_expand ? (uint64_t)file_size : speckage.min_memory_on_expand;
        char*    next_data = (char*)malloc(speckage.data_size + next_size);
        if (!next_data)
            return false;

        std::memcpy(next_data, speckage.data, speckage.data_size);

        if (speckage.data_size > 0)
        {
            free(speckage.data);
        }

        speckage.data = next_data;
        speckage.data_size += next_size;
    }

    file.read(reinterpret_cast<char*>(speckage.data) + speckage.first_empty_location, file_size);

    if (file.fail())
    {
        // io error
        file.close();
        return false;
    }

    speckage.file_info[std::string(filepath)] = std::make_pair(speckage.first_empty_location, file_size);

    file.close();
    speckage.first_empty_location += file_size;
    return true;
}

bool save_speckage_to_file(const speckage_for_write& toSave, const std::string& filepath)
{
    std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::trunc);

    write_speckage_to_stream(toSave, file);

    if (file.fail())
    {
        return false;
    }
    return true;
}

void unload_speckage(speckage_for_write& speckage_to_unload)
{
    free(speckage_to_unload.data);
    speckage_to_unload.file_info.clear();
    speckage_to_unload.data_size            = 0;
    speckage_to_unload.first_empty_location = 0;
    speckage_to_unload.data                 = nullptr;
}
}; // namespace speck_write