#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <queue>
#include "speck.hpp"
int main(int argc, char *argv[]){

	if(argc <= 1)
	{
		std::cout << "No arguments used. Run with argument \"help\" to see possible arguments." << std::endl;
		return -1;
	}

	if(std::strcmp(argv[1], "help") == 0)
	{
		//print help text
		//TODO do
		std::cout << "help text not yet written. Pls refer to main.cpp" << std::endl;
		return 0;
	}

	char* output = nullptr;
	std::vector<char*> files;
	char* directory = nullptr;

	//0 = undefined
	//1 = file input
	//2 = directory input
	//3 = output
	uint8_t mode = 0;

	for(int i = 1; i < argc; i++)
	{
		if(std::strcmp(argv[i], "-i") == 0)
		{
			mode = 1;
			continue;
		}else if(std::strcmp(argv[i], "-d") == 0)
		{
			mode = 2;
			continue;
		}else if (std::strcmp(argv[i], "-o") == 0) {
			mode = 3;
			continue;
		}

		if (mode == 0) {
			std::cout << "Incorrect arguments used. Run with argument \"help\" to see possible arguments." << std::endl;
			return -1;
		}

		switch (mode) {
		case 1:
			{
				char* file = (char*) malloc(std::strlen(argv[i]));
				std::strcpy(file, argv[i]);
				files.push_back(file);
			}
			break;
		case 2:
			if(directory != nullptr){
				std::cout << "Can only package one directory" << std::endl;
				return -1;
			}
			directory = (char*) malloc(std::strlen(argv[i]));
			std::strcpy(directory, argv[i]);
			break;
		case 3:
			if (output != nullptr) {
				std::cout << "Can only have one output" << std::endl;
				return -1;
			}
			output = (char*) malloc(std::strlen(argv[i]));
			std::strcpy(output, argv[i]);
			break;
		}
	}

	speck::speckage speckage;

	//file mode
	if(directory == nullptr){
		for(const char* filepath : files)
		{
			speck::addFileToPackage(speckage, filepath);
		}
	}else{
		std::filesystem::path path = std::string(directory);
		if(!std::filesystem::is_directory(path)){
			std::cout << "directory needs to be a directory" << std::endl;
			return -1;
		}
		std::filesystem::recursive_directory_iterator it(path, std::filesystem::directory_options::follow_directory_symlink);
		for(const auto& entry : it)
		{
			if(entry.is_directory()){
				continue;
			}
			speck::addFileToPackage(speckage, entry.path().u8string().c_str());
		}
	}

	speck::savePackageToFile(speckage, output);
}
