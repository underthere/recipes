cmake_minimum_required(VERSION 3.27)
project(recipes)

set(CMAKE_CXX_STANDARD 23)

set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/asio/asio/include)
set(expected_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/expected/include)

add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/async_simple)


include(cmake/common.cmake)
include_directories(include)

add_subdirectory(vthread)
