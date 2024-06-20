cmake_minimum_required(VERSION 3.27)
project(recipes)

set(CMAKE_CXX_STANDARD 23)

set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/asio/asio/include)

include_directories(include)

add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/async_simple)
add_subdirectory(coro)

message(STATUS "GTest include dir: ${gtest_SOURCE_DIR}/include")
message(STATUS "GMock include dir: ${gmock_SOURCE_DIR}/include")
set(gtest_INCLUDE_DIR ${gtest_SOURCE_DIR}/include)
set(gmock_INCLUDE_DIR ${gmock_SOURCE_DIR}/include)

add_executable(test_nalu_parse tests/test_nalu_parse.cpp)
add_executable(test_coroutine tests/test_coroutine.cpp)
