#include <iostream>
#include <stdio.h>
#include "speck.hpp"
int main(int argc, char *argv[]){
	auto out = speck::readPackageFromFile(R"(test2.speck)");
	printf("Hello Speck!");
	uint64_t size;
	std::cout << reinterpret_cast<char*>(readFileFromPackage(out, "fileno2", size));
	std::cout << reinterpret_cast<char*>(readFileFromPackage(out, "abcd", size));
}
