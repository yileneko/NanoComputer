cmake_minimum_required(VERSION 3.16)
project(NanoComputer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(nanocomputer
    src/main.cpp
    src/cpu.cpp
    src/memory.cpp
    src/assembler.cpp
)

target_include_directories(nanocomputer PRIVATE include)
