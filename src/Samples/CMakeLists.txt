# Define the samples
set(SAMPLE_FILES sample1.c)

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