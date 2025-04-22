
#include "speck.hpp"
#include <algorithm>
#include <cargs.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>

static struct cag_option options[] = {
    {.identifier = 'a', .access_letters = NULL, .access_name = "alloc-size", .value_name = "BYTES", .description = "Minimum number of bytes to allocate, when more memory needs to be allocated."},

    {.identifier = 'p', .access_letters = "a", .access_name = "append", .description = "When flag is set, speckage will be appended to the end of the file specified by out or appended speckages will be read, when r is set."},

    {.identifier = 'r', .access_letters = "r", .access_name = "read", .value_name = "FILE_PATH", .description = "Path to speck file to read"},

    {.identifier = 'i', .access_letters = "i", .access_name = "input", .value_name = "FILE_PATH", .description = "Path to file to speck. Ignored, if -r is set."},

    {.identifier = 'd', .access_letters = "d", .access_name = "input_dir", .value_name = "DIRECTORY_PATH", .description = "Path to directory to speck. Ignored, if -r is set."},

    {.identifier = 'o', .access_letters = "o", .access_name = "out", .value_name = "FILENAME", .description = "Output path. If -r is set, path to directory to unspeck the files."},

    {.identifier = 'b', .access_letters = "b", .access_name = "backslash", .description = "forces all slashes in saved filename to be backslashes."},

    {.identifier = 'f', .access_letters = "f", .access_name = "forward_slash", .description = "forces all slashes in saved filename to be forward slashes."},

    {.identifier = 'h', .access_letters = "h", .access_name = "help", .description = "Shows the command help"},

    {.identifier = 'n', .access_letters = "n", .access_name = "name", .value_name = "NAME", .description = "Name to give the speckage. Only used when creating a speckage. NOTE: this is not the output filename, see -o."},

    {.identifier     = 's',
     .access_letters = "s",
     .access_name    = "skip_beginning",
     .description    = "When appending, skips adding an empty header before the appended speckages. Use this when appending to a file that already has appended speckages."}
};

speck::speckage read_speckage(std::filesystem::path input)
{
    speck::speckage speckage = speck::read_speckage_from_file(input.string());

    if (speckage.data_size == 0)
    {
        std::printf("[ERROR] Invalid speckage");
        return speckage;
    }

    std::string filename_label  = "filename";
    uint16_t    max_name_length = filename_label.length();
    for (auto mapentry : speckage.file_info)
    {
        max_name_length = std::max(max_name_length, (uint16_t)mapentry.first.length());
    }

    std::printf("%-*s | %s\n", max_name_length, filename_label.c_str(), "size");

    for (const auto mapentry : speckage.file_info)
    {
        std::printf("%-*s | 0x%x\n", max_name_length, mapentry.first.c_str(), mapentry.second.second);
    }
    return speckage;
}

void unspeck(const speck::speckage& speckage, std::filesystem::path output_dir)
{
    std::printf("\n\nUnspecking into %s...\n", output_dir.c_str());

    for (const auto file_info : speckage.file_info)
    {
        uint64_t size;
        char*    location  = speck::read_file_from_speckage(speckage, file_info.first, size);
        auto     save_file = output_dir / std::filesystem::path(file_info.first);

        std::printf("\t-%s\n", save_file.c_str());
        if (!std::filesystem::exists(save_file.parent_path()))
        {
            std::filesystem::create_directories(save_file.parent_path());
            std::printf("\t\t-created directory %s\n", save_file.parent_path().c_str());
        }

        std::ofstream file(save_file, std::ios::out | std::ios::binary | std::ios::trunc);
        file.write(location, size);
        file.close();
    }
}

int main(int argc, char* argv[])
{
    std::vector<std::filesystem::path> input_files;
    std::vector<std::filesystem::path> input_directories;
    std::string                        out_speckage_name = "";
    bool                               output_is_set     = false;
    std::filesystem::path              output_file;
    bool                               should_append               = false;
    bool                               skip_append_security_footer = false;
    bool                               read_mode                   = false;
    std::filesystem::path              read_file;
    uint64_t                           min_expand          = 0;
    bool                               force_forward_slash = false;
    bool                               force_backslash     = false;

    cag_option_context                 context;
    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    if (argc == 1)
    {
        printf("Usage: speck [OPTION]...\n");
        printf("Creates speckages.\n\n");
        cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
        return EXIT_SUCCESS;
    }
    while (cag_option_fetch(&context))
    {
        switch (cag_option_get_identifier(&context))
        {
        case 'a': {
            char* end  = nullptr;
            min_expand = std::strtoull(cag_option_get_value(&context), &end, 10);
        }
        break;
        case 'p': {
            should_append = true;
            break;
        }
        case 's': {
            skip_append_security_footer = true;
            break;
        }
        case 'r':
            if (read_mode)
            {
                std::cout << "[ERROR] Only one read file may be set." << std::endl;
                return EXIT_FAILURE;
            }
            read_mode = true;
            read_file = cag_option_get_value(&context);
            break;
        case 'i':
            input_files.emplace_back(cag_option_get_value(&context));
            break;
        case 'd':
            input_directories.emplace_back(cag_option_get_value(&context));
            break;
        case 'o':
            if (output_is_set)
            {
                std::cout << "[ERROR] Only one output may be set." << std::endl;
                return EXIT_FAILURE;
            }
            output_is_set = true;
            output_file   = cag_option_get_value(&context);
            break;
        case 'f':
            if (force_backslash)
            {
                std::cout << "[ERROR] Only one slash style may be set." << std::endl;
                return EXIT_FAILURE;
            }
            force_forward_slash = true;
            break;
        case 'b':
            if (force_forward_slash)
            {
                std::cout << "[ERROR] Only one slash style may be set." << std::endl;
                return EXIT_FAILURE;
            }
            force_backslash = true;
            break;
        case 'n':
            out_speckage_name = cag_option_get_value(&context);
            break;
        case 'h':
            printf("Usage: speck [OPTION]...\n");
            printf("Creates speckages.\n\n");
            cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
            return EXIT_SUCCESS;
        case '?':
            cag_option_print_error(&context, stdout);
            break;
        }
    }

    // read speckage to output
    if (read_mode)
    {
        if (!should_append)
        {
            // single speckage file
            auto speckage = read_speckage(read_file);
            if (output_is_set)
            {
                unspeck(speckage, output_file);
            }
            speck::unload_speckage(speckage);
            return EXIT_SUCCESS;
        } else
        {
            // appended speckages
            auto                     speckages_infos = speck::discover_appended_speckages(read_file.string());
            std::vector<std::string> speckage_names;
            printf("found the following %i speckages:\n", speckages_infos.speckage_offset_map.size());
            for (const auto& [key, val] : speckages_infos.speckage_offset_map)
            {
                printf("\t-%s\n", key.c_str());
                speckage_names.push_back(key);
            }
            auto speckages = speck::read_appended_speckages_from_file(speckage_names, speckages_infos);
            if (output_is_set)
            {
                for (const auto& speckage : speckages)
                {
                    unspeck(speckage, output_file / speckage.name);
                }
            }

            return EXIT_SUCCESS;
        }
    }

    // gather all files in a vector
    for (const auto directory : input_directories)
    {
        if (!std::filesystem::is_directory(directory))
        {
            std::cout << "[ERROR] -d " << directory.string() << " needs to be a directory" << std::endl;
            return EXIT_FAILURE;
        }
        std::filesystem::recursive_directory_iterator it(directory, std::filesystem::directory_options::follow_directory_symlink);
        for (const auto& entry : it)
        {
            if (entry.is_directory())
            {
                continue;
            }
            input_files.push_back(entry);
        }
    }

    if (input_files.empty())
    {
        std::cout << "[ERROR] no input files were supplied" << std::endl;
        return EXIT_FAILURE;
    }

    // specking mode
    if (!should_append)
    {
        speck::speckage speckage;
        speckage.min_memory_on_expand = min_expand;

        if (out_speckage_name.empty())
        {
            if (!input_directories.empty())
            {
                out_speckage_name = input_directories[0].string();
            } else if (!input_files.empty())
            {
                out_speckage_name = input_files[0].string();
            }
        }

        speckage.name = out_speckage_name;

        for (const auto filepath : input_files)
        {
            std::string path = filepath.string();
            if (force_backslash)
                std::ranges::replace(path, '/', '\\');
            if (force_forward_slash)
                std::ranges::replace(path, '\\', '/');
            if (!speck::add_file_to_speckage(speckage, path))
            {
                std::cout << "[ERROR] " << path << " could not be specked" << std::endl;
                return EXIT_FAILURE;
            }
        }

        speck::save_speckage_to_file(speckage, output_file.string());
        return EXIT_SUCCESS;
    }

    // append mode
    if (should_append)
    {
        std::vector<speck::speckage> speckages;
        for (const auto& file : input_files)
        {
            speck::speckage read = speck::read_speckage_from_file(file.string());
            if (read.data_size > 0)
            {
                speckages.push_back(read);
            }
        }
        if (!speck::append_speckages_to_file(speckages, output_file.string(), !skip_append_security_footer))
        {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
}
