cmake_minimum_required(VERSION 2.8)
project(PAAV15_Project)

# Define the samples
set(SAMPLE_FILES samples.c test1.c test2.c test3.c test4.c test5.c test6.c test7.c test8.c test9.c test10.c test11.c test12.c)

# And compile the samples
add_custom_target(BuildSamples)
foreach(_file ${SAMPLE_FILES})
    set(CUR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${_file})
    string(REPLACE ".c" ".bc" BITCODE_FILE ${CUR_FILE})

    add_custom_command(
            COMMAND clang-3.6 -cc1 ${CUR_FILE} -I/usr/include/clang/3.6/include -I/usr/include/`arch`-linux-gnu -I/usr/include/ -v -emit-llvm -g -O0 -o ${BITCODE_FILE}
            DEPENDS ${CUR_FILE}
            TARGET BuildSamples)
endforeach()