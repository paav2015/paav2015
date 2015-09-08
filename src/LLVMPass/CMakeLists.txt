cmake_minimum_required(VERSION 3.1)
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
set(SOURCE_FILES IndependentLoopPass.cpp IndependentLoopPass.h MultiBlockInstIterator.cpp MultiBlockInstIterator.h)
# Output a shared library (.so) to be based on the compiled source files
add_library(IndependentLoop SHARED ${SOURCE_FILES})

# Define the samples
set(SAMPLE_FILES sample1.c zlib/gzlib.c zlib/adler32.c zlib/zutil.c zlib/compress.c zlib/crc32.c)

# And compile the samples
add_custom_target(BuildSamples)
foreach(_file ${SAMPLE_FILES})
        string(REPLACE ".c" ".bc" BITCODE_FILE ${SAMPLES_DIR}/${_file})
            #-I/usr/include/linux
        add_custom_command(
                COMMAND clang-3.6 -cc1 ${SAMPLES_DIR}/${_file} -I/usr/include/clang/3.6/include -I/usr/include/`arch`-linux-gnu -I/usr/include/ -v -emit-llvm -g -O0 -o ${BITCODE_FILE}
                DEPENDS ${SAMPLES_DIR}/${_file}
                TARGET BuildSamples)
        #add_custom_command(
        #        COMMAND llvm-gcc ${SAMPLES_DIR}/${_file} -emit-llvm -c -g -O0  -o ${BITCODE_FILE}
        #        DEPENDS ${SAMPLES_DIR}/${_file}
        #        TARGET BuildSamples)
        #message(WARNING "Added file: ${SAMPLES_DIR}/${_file} -> ${BITCODE_FILE}")
        #message(WARNING "llvm-gcc ${SAMPLES_DIR}/${_file} -emit-llvm -c -g -O0 -o ${BITCODE_FILE}")
endforeach()

add_dependencies(IndependentLoop BuildSamples)