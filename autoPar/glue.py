import subprocess
import fnmatch
import os
import logging

'''
clang -O0 -c -emit-llvm ./src/Samples/sample1.c -o ./src/Samples/sample1.cb
clang-3.6 -cc1 ${CUR_FILE} -I/usr/include/clang/3.6/include -I/usr/include/`arch`-linux-gnu -I/usr/include/ -v -emit-llvm -g -O0 -o ${BITCODE_FILE}
opt-3.6 -load ../src/LLVMPass/libIndependentLoop.so -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop ./src/Samples/sample1.bc -o /dev/null 2> ./autoPar/raw_input.txt
'''

class Gluer(object):
    '''
    classdocs
    '''


    def __init__(self):
        pass


    def collectAllFiles(self, rootDir):
        matches = []
        for root, dirnames, filenames in os.walk(rootDir):
          for filename in fnmatch.filter(filenames, '*.c'):
            matches.append(os.path.join(root, filename))
        return matches
    
    def genBitcodeForAllFiles(self, files):
        for file in files:
            bitcodeFilename =file[:-1] + "bc"
            commandString = "clang-3.6 -cc1 " +file +"  -I/usr/include/clang/3.6/include -I/usr/include/`arch`-linux-gnu -I/usr/include/ -v -emit-llvm -g -O0 -o " + bitcodeFilename
            try:
		logging.info('Running %s', commandString)
                os.system(commandString)
            except:
                logging.info('failed to builed %s', file)
            

    def genAllOutput(self,files):
        for file in files:
            bitcodeFilename =file[:-1] + "bc"
            commandString = "opt-3.6 -load ../src/LLVMPass/libIndependentLoop.so -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop "+bitcodeFilename + "  -o /dev/null 2>> ./raw_input.txt"
            logging.info('Running %s', bitcodeFilename)
            try:
                output = subprocess.check_output(commandString, shell=True)
            except:
                logging.info('failed to analize %s', file)
 
