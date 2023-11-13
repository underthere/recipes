cmake_minimum_required(VERSION 3.27)
project(recipes)

set(CMAKE_CXX_STANDARD 20)

add_library(recipes SHARED library.cpp)

set(EXE test_merge_tree)
add_executable(${EXE} tests/test_merge_tree.cpp)
add_test(NAME ${EXE} COMMAND ${EXE})