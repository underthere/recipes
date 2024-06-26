cmake_minimum_required(VERSION 3.18)
project(vthread LANGUAGES CXX ASM C)

set(CMAKE_CXX_STANDARD 20)


file(GLOB vthread_asm_src "context/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/*.S")


add_executable(vthread
        ${vthread_asm_src}
        main.cpp
        vcontext.cpp
        vcontext.h)