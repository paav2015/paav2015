
import fnmatch
import os

'''
clang -O0 -c -emit-llvm ./src/Samples/sample1.c -o ./src/Samples/sample1.cb
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
            commandString = "clang -O0 -c -emit-llvm " +file +" -o " + bitcodeFilename
            os.system(commandString)

    def genAllOutput(self,files):
        for file in files:
            bitcodeFilename =file[:-1] + "bc"
            commandString = "opt-3.6 -load ../src/LLVMPass/libIndependentLoop.so -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -instcombine -indvars -indloop "+bitcodeFilename + "  -o /dev/null 2>> ./raw_input.txt"
