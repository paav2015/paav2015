'''
Created on Sep 4, 2015

@author: yogev.vaknin
'''
import logging

from  lineInject import LineInjector
from  injectPlanner import InjectPlanner
from  injectPlanner import PlanType
import argparse
import sys
import os
import shutil

from optparse import OptionParser

def renameAllOrigFiles(fileOfLoops):
    loopLine = open(fileOfLoops,'r').readlines()
    handledFiles = set()
    for line in loopLine:
        fileName = line.split(':')[0]
        if fileName not in handledFiles:
            logging.debug('moving %s to %s', fileName, fileName + "_orig")
            os.rename(fileName, fileName + "_orig")
            handledFiles.add(fileName)

def revertAllOrigFiles(fileOfLoops):
    loopLine = open(fileOfLoops,'r').readlines()
    handledFiles = set()
    for line in loopLine:
        fileName = line.split(':')[0]
        if fileName not in handledFiles:
            logging.debug('moving %s to %s', fileName + "_orig", fileName)
            shutil.move(fileName + "_orig",fileName)
            handledFiles.add(fileName)

def main(argv=None):
    debug = True
    if (debug):
        logging.getLogger().setLevel(logging.DEBUG)
    else:
        logging.getLogger().setLevel(logging.INFO)

    program_name = os.path.basename(sys.argv[0])

    if argv is None:
        argv = sys.argv[1:]

    # setup option parser
    parser = OptionParser()
    parser.add_option("-r", "--run_cmd", dest="run_cmd", help="cmd for running the benchmark")
    parser.add_option("-b", "--build_cmd", dest="build_cmd", help="cmd for building the program (should support openML lib")
    

    # process options
    (opts, args) = parser.parse_args(argv)

        
    print("run_cmd = %s" % opts.run_cmd)
    print("build_cmd = %s" % opts.build_cmd)
    
    
    lineInj = LineInjector()
    injPlanner = InjectPlanner()
    
    
    injPlanner.reworkLoopFile("raw_input.txt","input.txt")
    renameAllOrigFiles("input.txt")
    
    injPlan = injPlanner.plan("input.txt", PlanType.noPar)
    logging.debug('plan no par %s', injPlan)
    for fileName in injPlan:
        lineInj.inject(fileName, fileName[:-5],injPlan[fileName])
        if (debug):
            shutil.copy(fileName[:-5], fileName[:-5] + "_no_par")
    os.system(opts.build_cmd)
    os.system(opts.run_cmd + " > out_no_par.txt ")
    
    revertAllOrigFiles("input.txt")
    renameAllOrigFiles("input.txt")
    
    injPlan = injPlanner.plan("input.txt", PlanType.par)
    logging.debug('plan par %s', injPlan)
    for fileName in injPlan:
        lineInj.inject(fileName, fileName[:-5],injPlan[fileName])
        if (debug):
            shutil.copy(fileName[:-5], fileName[:-5] + "_par")
    os.system(opts.build_cmd)
    os.system(opts.run_cmd + " > out_par.txt ")
    
    injPlanner.calcOut("input.txt", "out_no_par.txt" ,  "out_par.txt", "new_file.txt")
    revertAllOrigFiles("input.txt")
    renameAllOrigFiles("input.txt")

    injPlan = injPlanner.plan("input.txt", PlanType.final)
    logging.debug('plan final %s', injPlan)
    for fileName in injPlan:
        lineInj.inject(fileName, fileName[:-5],injPlan[fileName])

if __name__ == '__main__':
    main()
