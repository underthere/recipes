cmake_minimum_required(VERSION 3.27)
project(coro)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(thirdparty/async_simple)

add_library(coro Task.cpp)
target_link_libraries(coro PRIVATE async_simple_header_only)

add_executable(play_coro play_coro.cpp
        ThreadPool.hpp
        ThreadPool.cpp
        ThreadPoolExecutor.cpp
        ThreadPoolExecutor.h
        expected_wrapper.h
        error_code.h
        Awaitable.h)
target_link_libraries(play_coro PRIVATE async_simple_header_only)
