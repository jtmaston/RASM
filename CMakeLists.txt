cmake_minimum_required(VERSION 3.5)
project(rasm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_executable(rasm
    Assembler/src/assembler.cpp
)
target_include_directories(rasm PRIVATE
    Assembler/inc
)

