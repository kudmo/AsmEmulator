cmake_minimum_required(VERSION 3.30)
project(AsmEmulator)

set(CMAKE_CXX_STANDARD 23)

add_executable(AsmEmulator main.cpp
        Addressing.h
        Memory.h
        Instruction.h)
