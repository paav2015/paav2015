#!/bin/sh

#opt-3.6 -load ./bin/Debug/libIndependentLoop.so -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop ./src/Samples/sample1.bc -o /dev/null
opt-3.6 -load ./bin/Debug/libIndependentLoop.so -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop ./benchmark/tinyjpeg/seq/tinyjpeg-parse.ll -o /dev/null

