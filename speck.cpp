#include "include/speck.hpp"

namespace speck {
void write_speckage_to_stream(const speckage& speckage, std::ostream& stream)
{
    uint64_t header_size = sizeof(uint64_t); // starting size for the size itself

    uint8_t  name_size   = (uint8_t)speckage.name.size()+1;
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
        for (const auto& character : info.first)
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
}

bool read_header_from_stream(std::istream& stream, std::string& speckage_name, uint64_t& data_size, std::unordered_map<std::string, std::pair<uint64_t, uint64_t>>& file_info)
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
    auto header_size = get_data_from_stream<uint64_t>(stream) + SPECK_STRING_LENGTH + SPECK_VERSION_LENGTH;
    auto name_size   = get_data_from_stream<uint8_t>(stream);
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
        data_size += length;

        file_info[final_name] = std::pair(begin_offset, length);
    }
    assert((size_t)stream.tellg() == start_position + header_size);
    return true;
}

speckage read_speckage_from_stream(std::istream& stream)
{
    speckage out;

    if (!read_header_from_stream(stream, out.name, out.data_size, out.file_info))
    {
        return out;
    }

    out.first_empty_location = out.data_size + 1;
    out.data                 = (char*)malloc(out.data_size);
    stream.read(reinterpret_cast<char*>(out.data), out.data_size);

    return out;
}

appendedSpeckageInfos discover_appended_speckages(const std::string& filepath)
{
    auto                  file = std::ifstream(filepath, std::ios::binary | std::ios::in);
    appendedSpeckageInfos out;
    if (file.bad())
        return out;
    out          = discover_appended_speckages(file);
    out.filepath = filepath;
    file.close();
    return out;
}

appendedSpeckageInfos discover_appended_speckages(std::istream& stream)
{
    appendedSpeckageInfos out = {};
    out.filepath              = "";

    stream.seekg(0, std::ios_base::end);
    size_t position = stream.tellg();

    while (!stream.bad() && !stream.fail())
    {

        // check post-header postfix
        char name_string[SPECK_FOOTER_STRING_LENGTH];
        char version_string[SPECK_VERSION_LENGTH];

        position -= SPECK_FOOTER_STRING_LENGTH;
        stream.seekg(position, std::ios_base::beg);
        stream.read(name_string, SPECK_FOOTER_STRING_LENGTH);

        position -= SPECK_VERSION_LENGTH;
        stream.seekg(position, std::ios_base::beg);
        stream.read(version_string, SPECK_VERSION_LENGTH);

        // if post-header is invalid, all appended speckages have been found
        if (std::strcmp(name_string, SPECK_FOOTER_STRING) != 0 || std::strcmp(version_string, SPECK_VERSION) != 0)
        {
            return out;
        }

        // speckage name
        position -= sizeof(uint8_t);
        stream.seekg(position, std::ios_base::beg);
        uint8_t name_length = 0;
        stream.read((char*)&name_length, sizeof(name_length));

        std::string name;
        name.resize(name_length);
        position -= name_length;
        stream.seekg(position, std::ios_base::beg);
        stream.read(name.data(), name_length);

        // read distance to header
        uint64_t size = 0;
        position -= sizeof(size);
        stream.seekg(position, std::ios_base::beg);
        stream.read((char*)&size, sizeof(size));

        position -= size;

        out.speckage_offset_map[name] = position;

        std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info;
        uint64_t                                                       data_size;
        std::string                                                    temp;
        read_header_from_stream(stream, temp, data_size, file_info);

        for (auto const& [file_name, val] : file_info)
        {
            out.filename_speckage_map[file_name] = name;
        }
    }

    return out;
}

speckage read_appended_speckage_from_file(const std::string& speckage_name, const appendedSpeckageInfos& appended_speckage_infos)
{
    auto     file = std::ifstream(appended_speckage_infos.filepath, std::ios::binary | std::ios::in);
    speckage out;
    if (file.bad())
        return out;
    out = read_appended_speckage_from_stream(speckage_name, appended_speckage_infos, file);
    file.close();
    return out;
}

std::vector<speckage> read_appended_speckages_from_file(const std::vector<std::string>& speckage_names, const appendedSpeckageInfos& appended_speckage_infos)
{
    auto                  file = std::ifstream(appended_speckage_infos.filepath, std::ios::binary | std::ios::in);
    std::vector<speckage> out;
    if (file.bad())
        return out;
    out = read_appended_speckages_from_stream(speckage_names, appended_speckage_infos, file);
    file.close();
    return out;
}

speckage read_appended_speckage_from_stream(const std::string& speckage_name, const appendedSpeckageInfos& appended_speckage_infos, std::istream& stream)
{
    stream.seekg(appended_speckage_infos.speckage_offset_map.at(speckage_name), std::ios_base::beg);
    return read_speckage_from_stream(stream);
}

std::vector<speckage> read_appended_speckages_from_stream(const std::vector<std::string>& speckage_names, const appendedSpeckageInfos& appended_speckage_infos, std::istream& stream)
{
    std::vector<speckage> out;
    for (const auto& name : speckage_names)
    {
        stream.seekg(appended_speckage_infos.speckage_offset_map.at(name), std::ios_base::beg);
        out.push_back(read_speckage_from_stream(stream));
    }
    return out;
}

bool append_speckages_to_file(const std::vector<speckage>& speckages, const std::string& filepath)
{
    std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::ate | std::ios::app);

    append_speckages_to_stream(speckages, file);

    if (file.fail())
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool append_speckages_to_stream(const std::vector<speckage>& speckages, std::ostream& stream)
{
    stream.seekp(0, std::ios_base::end);

    for (int i = 0; i < SPECK_FOOTER_STRING_LENGTH + SPECK_VERSION_LENGTH; ++i)
    {
        stream.put(0);
    }

    for (const auto& speckage : speckages)
    {
        size_t position = stream.tellp();
        write_speckage_to_stream(speckage, stream);
        uint64_t size = (size_t)stream.tellp() - position;

        // now write footer
        stream.write((char*)&size, sizeof(size)); // size of speckage
        uint8_t name_size = (uint8_t)speckage.name.size();
        stream.write(speckage.name.data(), name_size);                 // name
        stream.write((char*)&name_size, sizeof(name_size));            // name size
        stream.write(SPECK_VERSION, SPECK_VERSION_LENGTH);             // version
        stream.write(SPECK_FOOTER_STRING, SPECK_FOOTER_STRING_LENGTH); // footer postfix
    }

    return true;
}

bool add_file_to_speckage(speckage& speckage, const std::string& filepath)
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

bool save_speckage_to_file(const speckage& toSave, const std::string& filepath)
{

    std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::trunc);

    write_speckage_to_stream(toSave, file);

    if (file.fail())
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}

speckage read_speckage_from_file(std::string filepath)
{
    auto file = std::ifstream(filepath, std::ios::binary | std::ios::in);
    if (file.bad())
        return {};
    speckage out = read_speckage_from_stream(file);
    file.close();
    return out;
}

void unload_speckage(speckage& speckage_to_unload)
{
    free(speckage_to_unload.data);
    speckage_to_unload.file_info.clear();
    speckage_to_unload.data_size            = 0;
    speckage_to_unload.first_empty_location = 0;
    speckage_to_unload.data                 = nullptr;
}

std::vector<std::string> discover_files_in_speckage(const std::string& filepath)
{
    auto                     file = std::ifstream(filepath, std::ios::binary | std::ios::in);
    std::vector<std::string> out;
    if (file.bad())
        return out;

    std::string speckage_name;
    uint64_t data_size;
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> file_info;
    read_header_from_stream(file, speckage_name, data_size, file_info);

    for(const auto& [name, val] : file_info){
      out.push_back(name);
    }
    
    return out;
}

char* read_file_from_speckage(const speckage& speckage, std::string filepath, uint64_t& size)
{
    if (!(speckage.file_info.count(filepath)))
        return nullptr;
    std::pair<uint64_t, uint64_t> info = speckage.file_info.at(filepath);
    size                               = info.second;
    return speckage.data + info.first;
}
} // namespace speck
