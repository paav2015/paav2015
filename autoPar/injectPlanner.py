from timeStringGenerator import TimeStringGenerator
from itertools import izip
import logging
import re

class PlanType:
    noPar = 1
    par = 2
    final = 3



class InjectPlanner(object):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
    def __grep(self, fileName, matchString):
        with open(fileName) as f:
            for line in f:
                if matchString in line:
                    f.close()
                    return line
            f.close()
            return ""
    def getLineID(self, path, startLine):
        return re.sub(r'\W+', '', path) + str(startLine)

    def __isForLoop(self,fileName,lineNumber):
        f=open(fileName)
        lines=f.readlines()
        if "for" in lines[lineNumber-1]:
            logging.debug('find for, return True for file %s line num %s, line : %s', fileName,str(lineNumber -1),lines[lineNumber-1])
            f.close()
            return True
        else:
            logging.debug('find for, return False for file %s line num %s, line : %s', fileName,str(lineNumber -1),lines[lineNumber-1])
            f.close()
            return False


    def __containBracket(self,fileName,lineNumber,char):
        f=open(fileName)
        lines=f.readlines()
        if (char in lines[lineNumber-1]) or (lines[lineNumber-1].isspace()):
            logging.debug('find }, return True for file %s line num %s, line : %s', fileName,str(lineNumber -1),lines[lineNumber-1])
            f.close()
            return True
        else:
            logging.debug('find }, return False for file %s line num %s, line : %s', fileName,str(lineNumber -1),lines[lineNumber-1])
            f.close()
            return False


    def reworkLoopFile(self, fileOfLoops, newLoopFile):
        loopLine = open(fileOfLoops,'r').readlines()
        dest = open(newLoopFile,'w')
        for line in loopLine:
            fileName = line.split(':')[1]
            startLine = int(line.split(':')[2])
            endLine = int(line.split(':')[3])
            if not self.__isForLoop(fileName,startLine):
                if self.__isForLoop(fileName,startLine-1):
                    startLine = startLine-1
                else:
                    logging.debug('for file %s , didnt find for, line %s,%s', fileName,str(startLine),str(startLine-1))
                    continue

            endLine = startLine
            foundOpen = False
            numOfOpen = 0
            print "-----------------------start search -----"
            print "line is" + str(endLine)
            if self.__containBracket(fileName, endLine,"}"):
                numOfOpen -= 1
                print "found  }:" + str(numOfOpen)
            if self.__containBracket(fileName, endLine,"{"):
                numOfOpen += 1
                print "found  {:" + str(numOfOpen)
                foundOpen = True
            try:
                while (numOfOpen != 0) or (foundOpen == False):
                    endLine += 1
                    if self.__containBracket(fileName, endLine,"}"):
                        numOfOpen -= 1
                        print "found  }:" + str(numOfOpen) +"line:" + str(endLine)
                    if self.__containBracket(fileName, endLine,"{"):
                        numOfOpen += 1
                        print "found  {:" + str(numOfOpen)  +"line:" + str(endLine)
                        foundOpen = True
            except:
                logging.debug('for file %s , didnt find }  line endline;%s', fileName,str(endLine))
                continue

            dest.write(fileName+":"+str(startLine)+":"+str(endLine)+":" + self.getLineID(fileName,startLine) + "\n")
        dest.close()

        
    def plan(self, fileOfLoops, planType):
        stringGen = TimeStringGenerator()
        insertPlanDic = {}
        loopLine = open(fileOfLoops,'r').readlines()
        for line in loopLine:
            fileName = line.split(':')[0]
            fileName += "_orig"
            startLine = int(line.split(':')[1])
            endLine = int(line.split(':')[2])

            if not self.__isForLoop(fileName,startLine):
                continue

            if fileName not in insertPlanDic:
                insertPlanDic[fileName] = {}
                if (planType != PlanType.final):
                    insertPlanDic[fileName][0] = stringGen.genStartFileString()
            if (planType == PlanType.par):
                insertPlanDic[fileName][startLine-1] =  stringGen.genStartTimeString(startLine) +stringGen.genOpenMPLoop() 
            elif(planType == PlanType.final):
                insertPlanDic[fileName][startLine-1] =  stringGen.genOpenMPLoop() 
            else:
                insertPlanDic[fileName][startLine-1] = stringGen.genStartTimeString(startLine)
            if (planType != PlanType.final):
                insertPlanDic[fileName][endLine-1] = stringGen.genEndTimeString(fileName, startLine, endLine) 
        return insertPlanDic
    
    def calcOut(self, orig, noPar ,  par, newfile):
        f = open(newfile, 'a')
        with open(noPar) as noParFile,open(par) as parFile:
            for lineNoPar in noParFile:
                print lineNoPar.split(':')
                if "loop" not in lineNoPar:
                    continue
                linePar = self.__grep(par, lineNoPar.split(':')[3])
                if(float(lineNoPar.split(':')[2]) <= float(linePar.split(':')[2])):
                    print lineNoPar.split(':')[2],lineNoPar.split(':')[3]
                    if (linePar.split(':')[3] != lineNoPar.split(':')[3]):
                        print "ERROR, id not match"
                        print lineNoPar.split(':')[3]
                        print linePar.split(':')[3]
                    newLine = self.__grep(orig, linePar.split(':')[3])
                    f.write(newLine)
        f.close()
