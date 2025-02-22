cmake_minimum_required(VERSION 3.16)

project(speck)
set(LIBRARY_SOURCE_FILES speck.hpp speck.cpp)
add_library(speck ${LIBRARY_SOURCE_FILES})
add_subdirectory(cargs)
add_executable(speckCLI main.cpp)
target_compile_features(speckCLI PRIVATE cxx_std_20)
target_link_libraries(speckCLI PRIVATE speck cargs)


set(CMAKE_CXX_STANDARD 20)
target_compile_features(speck PRIVATE cxx_std_20)
