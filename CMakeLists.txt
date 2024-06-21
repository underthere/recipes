cmake_minimum_required(VERSION 3.27)
project(recipes)

set(CMAKE_CXX_STANDARD 23)

set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/asio/asio/include)
set(expected_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/expected/include)

add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/async_simple)


include(cmake/common.cmake)
include_directories(include)

add_subdirectory(coro)
add_subdirectory(media/librtsp)
add_subdirectory(vthread)

add_executable(test_nalu_parse tests/test_nalu_parse.cpp)
add_executable(test_coroutine tests/test_coroutine.cpp)
add_executable(play_asio tests/play_asio.cpp)
target_include_directories(play_asio PRIVATE ${ASIO_INCLUDE_DIR} ${expected_INCLUDE_DIR})

add_executable(play_uthread tests/play_uthread.cpp)
target_link_libraries(play_uthread PRIVATE async_simple_header_only)