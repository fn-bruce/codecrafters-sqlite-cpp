cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(sqlite ${SOURCE_FILES})
