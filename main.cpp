#include <iostream>
#include <stdio.h>
#include "speck.hpp"
int main(int argc, char *argv[]){
	auto out = speck::readPackageFromFile("test.speck");
	printf("Hello Speck!");
	//std::cout << out;
}
