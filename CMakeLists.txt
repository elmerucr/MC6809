cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3")

project(test)

include_directories(
    src/
    test/
)

add_executable(test test/main.cpp src/mc6809.cpp)
