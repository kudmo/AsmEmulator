cmake_minimum_required(VERSION 3.30)
project(AsmEmulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-unused-parameter -std=c++2b -Wall -Wextra -g -fconcepts-diagnostics-depth=10")
add_executable(AsmEmulator main.cpp
        Addressing.h
        Memory.h
        Instruction.h)
