'''
Created on Sep 14, 2015

@author: yogev.vaknin
'''
from glue import Gluer

if __name__ == '__main__':
    glue = Gluer()
    files = glue.collectAllFiles("../bench/")
    print files
    glue.genBitcodeForAllFiles(files)
    glue.genAllOutput(files)
