cmake_minimum_required(VERSION 2.8)
project(PAAV15_Project)

# Extract compiler flags required to build LLVM
execute_process(COMMAND llvm-config-3.6 --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS)
# Compiler flags
set(CMAKE_CXX_FLAGS_TEMP "${CMAKE_CXX_FLAGS} -std=c++11 ${LLVM_CXX_FLAGS} -g")
# Gotta get rid of  the new-line character induced from the llvm-config command
string(REPLACE "\n" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_TEMP})

# For debugging purpose - print the C++ flags
#message(WARNING CXX Flags: '${CMAKE_CXX_FLAGS}')

# Generate a verbose makefile
set(CMAKE_VERBOSE_MAKEFILE true)

# List of source files
set(SOURCE_FILES IndependentLoopPass.cpp IndependentLoopPass.h)
# Output a shared library (.so) to be based on the compiled source files
add_library(IndependentLoop SHARED ${SOURCE_FILES})

add_dependencies(IndependentLoop BuildSamples)
