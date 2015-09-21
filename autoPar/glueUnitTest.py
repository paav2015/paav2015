'''
Created on Sep 14, 2015

@author: yogev.vaknin
'''

import logging

from glue import Gluer

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    glue = Gluer()
    files = glue.collectAllFiles("../benchmark/")
    print files
    glue.genBitcodeForAllFiles(files)
    glue.genAllOutput(files)
