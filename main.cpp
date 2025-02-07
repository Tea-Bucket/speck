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

speck::speckage read_speckage(std::filesystem::path input) {
  speck::speckage speckage = speck::read_speckage_from_file(input.string());

  if (speckage.data_size == 0) {
    std::printf("[ERROR] Invalid speckage");
    return speckage;
  }

  std::string filename_label = "filename";
  uint16_t max_name_length = filename_label.length();
  for (auto mapentry : speckage.file_info) {
    max_name_length =
        std::max(max_name_length, (uint16_t)mapentry.first.length());
  }

  std::printf("%-*s | %s\n", max_name_length, filename_label.c_str(), "size");

  for (const auto mapentry : speckage.file_info) {
    std::printf("%-*s | 0x%x\n", max_name_length, mapentry.first.c_str(),
                mapentry.second.second);
  }
  return speckage;
}

void unspeck(const speck::speckage &speckage, std::filesystem::path output_dir){
  std::printf("\n\nUnspecking into %s...\n", output_dir.c_str());
  
  for(const auto file_info : speckage.file_info){
    uint64_t size;
    char* location = speck::read_file_from_speckage(speckage, file_info.first, size);
    auto save_file = output_dir / std::filesystem::path(file_info.first);

    std::printf("\t-%s\n", save_file.c_str());
    if(!std::filesystem::exists(save_file.parent_path())){
      std::filesystem::create_directories(save_file.parent_path());
      std::printf("\t\t-created directory %s\n", save_file.parent_path().c_str());
    }
    
    std::ofstream file(save_file,
                     std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(location, size);
    file.close();
  }
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
    auto speckage = read_speckage(read_file);
    if(output_is_set){
      unspeck(speckage, output_file);
    }
    speck::unload_speckage(speckage);
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
    speck::add_file_to_speckage(speckage, filepath.c_str());
  }

  speck::save_speckage_to_file(speckage, output_file.c_str());
}
