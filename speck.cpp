#include "include/speck.hpp"

bool speck::add_file_to_speckage(speckage& speckage, const std::string& filepath)
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

bool speck::save_speckage_to_file(const speckage& toSave, const std::string& filepath)
{
    uint64_t header_size = 8;

    for (const auto& info : toSave.file_info)
    {
        // two uint64_t for each file;
        header_size += 16;

        header_size += info.first.size() + 1;
    }

    std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(SPECK_STRING, SPECK_STRING_LENGTH);
    file.write(SPECK_VERSION, SPECK_VERSION_LENGTH);
    file.write(reinterpret_cast<char*>(&header_size), 8);
    for (const auto& info : toSave.file_info)
    {
        for (const auto& character : info.first)
        {
            file << character;
        }
        file << '\0';
        // position
        file.write(reinterpret_cast<const char*>(&info.second.first), 8);
        // size
        file.write(reinterpret_cast<const char*>(&info.second.second), 8);
    }

    file.write(reinterpret_cast<char*>(toSave.data), toSave.first_empty_location);

    if (file.fail())
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}

speck::speckage speck::read_speckage_from_file(std::string filepath)
{
    auto     file = std::ifstream(filepath, std::ios::binary | std::ios::in);
    speckage out;
    if (file.bad())
        return out;

    // check header prefix
    char name_string[SPECK_STRING_LENGTH];
    char version_string[SPECK_VERSION_LENGTH];

    file.read(name_string, SPECK_STRING_LENGTH);
    file.read(version_string, SPECK_VERSION_LENGTH);

    if (std::strcmp(name_string, SPECK_STRING) != 0 || std::strcmp(version_string, SPECK_VERSION) != 0)
    {
        return out;
    }

    // read header
    auto header_size = get_data_from_stream<uint64_t>(file) + SPECK_STRING_LENGTH + SPECK_VERSION_LENGTH;
    while ((uint64_t)file.tellg() < header_size)
    {
        std::vector<char> name;
        char              temp;
        do
        {
            file.get(temp);
            name.push_back(temp);
        }
        while (temp);
        std::string final_name   = name.data();

        auto        begin_offset = get_data_from_stream<uint64_t>(file);

        auto        length       = get_data_from_stream<uint64_t>(file);
        out.data_size += length;
        out.first_empty_location  = out.data_size + 1;

        out.file_info[final_name] = std::pair(begin_offset, length);
    }
    assert((uint64_t)file.tellg() == header_size);
    out.data = (char*)malloc(out.data_size);
    file.read(reinterpret_cast<char*>(out.data), out.data_size);
    file.close();
    return out;
}

void speck::unload_speckage(speckage& speckage_to_unload)
{
    free(speckage_to_unload.data);
    speckage_to_unload.file_info.clear();
    speckage_to_unload.data_size            = 0;
    speckage_to_unload.first_empty_location = 0;
    speckage_to_unload.data                 = nullptr;
}

char* speck::read_file_from_speckage(const speckage& speckage, std::string filepath, uint64_t& size)
{
    if (!(speckage.file_info.count(filepath)))
        return nullptr;
    std::pair<uint64_t, uint64_t> info = speckage.file_info.at(filepath);
    size                               = info.second;
    return speckage.data + info.first;
}
