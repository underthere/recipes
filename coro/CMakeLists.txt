cmake_minimum_required(VERSION 3.27)
project(coro)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(thirdparty/async_simple)
set(ASIO_INCLUDE_DIR thirdparty/asio/asio/include)

add_compile_definitions(USE_ASIO)

add_library(coro Task.cpp
        channel/Channel.h)

target_link_libraries(coro PRIVATE async_simple_header_only)
target_include_directories(coro PUBLIC ${ASIO_INCLUDE_DIR} .)

add_executable(play_coro play_coro.cpp
        ThreadPool.hpp
        ThreadPool.cpp
        ThreadPoolExecutor.cpp
        ThreadPoolExecutor.h
        expected_wrapper.h
        error_code.h
        Awaitable.h)
target_include_directories(play_coro PUBLIC ${ASIO_INCLUDE_DIR} .)
target_link_libraries(play_coro PRIVATE async_simple_header_only)
