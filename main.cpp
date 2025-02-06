#include "speck.hpp"
#include <cargs.h>
#include <cstdio>
#include <filesystem>
#include <iostream>

static struct cag_option options[] = {
    {.identifier = 'r',
     .access_letters = "r",
     .access_name = "read",
     .value_name = "FILE_PATH",
     .description = "Path to speck file to read"},

    {.identifier = 'i',
     .access_letters = "i",
     .access_name = "input",
     .value_name = "FILE_PATH",
     .description = "Path to file to speck. Ignored, if -r is set."},

    {.identifier = 'd',
     .access_letters = "d",
     .access_name = "input_dir",
     .value_name = "DIRECTORY_PATH",
     .description = "Path to directory to speck. Ignored, if -r is set."},

    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "out",
     .value_name = "FILENAME",
     .description =
         "Output path. If -r is set, path to directory to unspeck the files."},

    {.identifier = 'h',
     .access_letters = "h",
     .access_name = "help",
     .description = "Shows the command help"}};

void read_speckage(std::filesystem::path input,
                   std::filesystem::path output_dir) {
  speck::speckage speckage = speck::readPackageFromFile(input.string());

  if (speckage.data_size == 0) {
    std::printf("[ERROR] Invalid speckage");
    return;
  }

  std::string filename_label = "filename";
  uint16_t max_name_length = filename_label.length();
  for (auto mapentry : speckage.file_info) {
    max_name_length =
        std::max(max_name_length, (uint16_t)mapentry.first.length());
  }

  std::printf("%-*s | %s\n", max_name_length, filename_label.c_str(), "size");

  for (auto mapentry : speckage.file_info) {
    std::printf("%-*s | 0x%x\n", max_name_length, mapentry.first.c_str(),
                mapentry.second.second);
  }

  speck::unloadSpeckage(speckage);
}

int main(int argc, char *argv[]) {

  std::vector<std::filesystem::path> input_files;
  std::vector<std::filesystem::path> input_directories;
  bool output_is_set = false;
  std::filesystem::path output_file;
  bool read_mode = false;
  std::filesystem::path read_file;

  cag_option_context context;
  cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&context)) {
    switch (cag_option_get_identifier(&context)) {
    case 'r':
      if (read_mode) {
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
      if (output_is_set) {
        std::cout << "[ERROR] Only one output may be set." << std::endl;
        return EXIT_FAILURE;
      }
      output_is_set = true;
      output_file = cag_option_get_value(&context);
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

  if (read_mode) {
    read_speckage(read_file, output_file);
    return EXIT_SUCCESS;
  }

  for (const auto directory : input_directories) {
    if (!std::filesystem::is_directory(directory)) {
      std::cout << "[ERROR] -d " << directory.string()
                << " needs to be a directory" << std::endl;
      return EXIT_FAILURE;
    }
    std::filesystem::recursive_directory_iterator it(
        directory,
        std::filesystem::directory_options::follow_directory_symlink);
    for (const auto &entry : it) {
      if (entry.is_directory()) {
        continue;
      }
      input_files.push_back(entry);
    }
  }

  speck::speckage speckage;

  for (const auto filepath : input_files) {
    speck::addFileToPackage(speckage, filepath.c_str());
  }

  speck::savePackageToFile(speckage, output_file.c_str());
}
