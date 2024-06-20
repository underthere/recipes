cmake_minimum_required(VERSION 3.27)
project(coro)

set(CMAKE_CXX_STANDARD 23)

message(STATUS "ASIO include dir: ${ASIO_INCLUDE_DIR}")

add_library(coro src/Task.cpp channel/Channel.h)

target_link_libraries(coro PRIVATE async_simple_header_only)
target_include_directories(coro PUBLIC ${ASIO_INCLUDE_DIR} .)

add_executable(play_coro src/play_coro.cpp
        src/ThreadPool.hpp
        src/ThreadPool.cpp
        src/ThreadPoolExecutor.cpp
        src/ThreadPoolExecutor.h
        src/expected_wrapper.h
        src/error_code.h
        src/Awaitable.h
        src/NoCopy.h
        src/RingBuffer.h)
target_include_directories(play_coro PUBLIC ${ASIO_INCLUDE_DIR} .)
target_link_libraries(play_coro PRIVATE async_simple_header_only)

option(ENABLE_TEST "Enable test" ON)
if (ENABLE_TEST)
    function(add_test_executable name)
        add_executable(${name} ${ARGN})
        target_include_directories(${name} PRIVATE . ${gtest_INCLUDE_DIR} ${gmock_INCLUDE_DIR})
        target_link_libraries(${name} PRIVATE gtest_main)
    endfunction()
    add_test_executable(test_ring_buffer tests/test_ring_buffer.cpp)

    add_executable(coro_ping_pong tests/test_coro_ping_pong.cpp)
    target_include_directories(coro_ping_pong PRIVATE . ${gtest_INCLUDE_DIR} ${gmock_INCLUDE_DIR} ${ASIO_INCLUDE_DIR})
    target_link_libraries(coro_ping_pong PRIVATE async_simple)
endif ()