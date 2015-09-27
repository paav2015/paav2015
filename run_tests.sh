#!/bin/sh
LIB_NAME=./bin/Debug/libIndependentLoop.so
SUPPORTING_PASSES="-basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars"
PASS_NAME="-indloop"
OPT=opt-3.6
TESTS_DIR=./src/Samples

for test_file in ${TESTS_DIR}/test*.bc
do
	echo "**** Running on ${test_file} ****"
	${OPT} -load ${LIB_NAME} ${SUPPORTING_PASSES} ${PASS_NAME} ${test_file} -o /dev/null
done

