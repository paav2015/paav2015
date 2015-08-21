#!/bin/sh

opt-3.6 -load ./bin/Debug/libIndependentLoop.so -basicaa -mem2reg -print-alias-sets -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop ./src/Samples/sample1.bc -o /dev/null

