#include <algorithm>
#include <iostream>
#include <stdio.h>
#include "speck.hpp"
int main(int argc, char *argv[]){
	//auto out = speck::readPackageFromFile(R"(test2.speck)");
	//std::cout << reinterpret_cast<char*>(readFileFromPackage(out, "fileno2", size));
	//std::cout << reinterpret_cast<char*>(readFileFromPackage(out, "abcd", size));
	speck::speckage test;
	speck::addFileToPackage(test, "test2.txt");
	speck::addFileToPackage(test, "test.txt");
	speck::savePackageToFile(test, "test3.speck");

	//std::cout << reinterpret_cast<char*>(readFileFromPackage(test, "test.txt", size));

	auto out = speck::readPackageFromFile(R"(test3.speck)");
	uint64_t size;
	std::cout << reinterpret_cast<char*>(readFileFromPackage(out, "test.txt", size));
	std::cout << size;
}
