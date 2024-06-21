cmake_minimum_required(VERSION 3.27)
project(vthread LANGUAGES CXX ASM C)

set(CMAKE_CXX_STANDARD 23)


add_executable(vthread main.cpp make_arm64_elf.asm)