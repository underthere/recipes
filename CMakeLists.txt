cmake_minimum_required(VERSION 3.27)
project(recipes)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

set(folly_DIR /Volumes/MacStorage/dev_deps/folly/lib/cmake/folly)
find_package(folly REQUIRED)
link_directories(/opt/homebrew/Cellar/icu4c/74.2/lib/)
link_directories(/opt/homebrew/Cellar/gflags/2.2.2/lib/)

message(STATUS "Folly include dir: ${FOLLY_INCLUDE_DIR}")
message(STATUS "Folly library: ${FOLLY_LIBRARIES}")


set(EXE test_merge_tree)
add_executable(${EXE} tests/test_merge_tree.cpp)
add_test(NAME ${EXE} COMMAND ${EXE})

add_executable(test_proxy tests/test_proxy.cc)
add_executable(test_nalu_parse tests/test_nalu_parse.cpp)
add_executable(test_coroutine tests/test_coroutine.cpp)
add_executable(test_execution tests/test_execution.cpp)
target_include_directories(test_execution PRIVATE ${FOLLY_INCLUDE_DIR})
target_link_libraries(test_execution PRIVATE ${FOLLY_LIBRARIES})

add_subdirectory(coro)

add_executable(play_asio tests/play_asio.cpp)