cmake_minimum_required(VERSION 3.27)
project(librtsp)

set(CMAKE_CXX_STANDARD 23)

message(STATUS "ASIO include dir: ${ASIO_INCLUDE_DIR}")

add_library(rtsp src/rtsp_protocol.cpp src/rtsp_protocol.h
        src/RtspServer.h)
target_include_directories(rtsp PUBLIC ${expected_INCLUDE_DIR} ${ASIO_INCLUDE_DIR})

option(ENABLE_RTSP_TEST "Enable RTSP Test" ON)
if (ENABLE_RTSP_TEST)
    add_test_executable(test_rtsp tests/test_rtsp.cpp)
    target_link_libraries(test_rtsp PRIVATE rtsp)
    target_include_directories(test_rtsp PRIVATE src)
endif ()
