cmake_minimum_required (VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3")

project(emulate_mc6809)

include_directories(
    src/
    test/
)

add_executable(
	emulate_mc6809
	test/main.cpp
	test/rom.cpp
	src/mc6809.cpp
	src/mc6809_disassembler.cpp
	src/mc6809_instructions.cpp
	src/mc6809_addressing_modes.cpp
)
