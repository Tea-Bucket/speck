#include <iostream>
#include <stdio.h>
#include "speck.hpp"
int main(int argc, char *argv[]){
	auto out = speck::readPackageFromFile(R"(C:\Users\Max\Documents\GitKraken Repos\speck\test.speck)");
	printf("Hello Speck!");
	std::cout << reinterpret_cast<char*>(out.data);
}
