#include "speck.hpp"
#include <cargs.h>
#include <cstdio>
#include <filesystem>
#include <iostream>

static struct cag_option options[] = {
    {.identifier = 'i',
     .access_letters = "i",
     .access_name = "input",
     .value_name = "FILE_PATH",
     .description = "Path to file to speck"},

    {.identifier = 'd',
     .access_letters = "d",
     .access_name = "input_dir",
     .value_name = "DIRECTORY_PATH",
     .description = "Path to directory to speck"},

    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "out",
     .value_name = "FILENAME",
     .description = "Output path"},

    {.identifier = 'h',
     .access_letters = "h",
     .access_name = "help",
     .description = "Shows the command help"}};

int main(int argc, char *argv[]) {

  std::vector<std::filesystem::path> input_files;
  std::vector<std::filesystem::path> input_directories;
  bool output_is_set = false;
  std::filesystem::path output_file;

  cag_option_context context;
  cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&context)) {
    switch (cag_option_get_identifier(&context)) {
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
