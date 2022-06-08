cmake_minimum_required(VERSION 3.5)
project(rasm VERSION 0.1 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_executable(rasm
    src/assembler.cpp
    inc/QR-Code-generator/cpp/qrcodegen.cpp
)
target_include_directories(rasm PRIVATE
    inc
    inc/QR-Code-generator/cpp
)
target_link_libraries(rasm ${OpenCV_LIBS})
