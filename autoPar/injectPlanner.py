from timeStringGenerator import TimeStringGenerator
from itertools import izip

class PlanType:
    noPar = 1
    par = 2
    final = 3



class InjectPlanner(object):
    '''
    classdocs
    '''
    print "huha"


    def __init__(self):
        '''
        Constructor
        '''
    def __isForLoop(self,fileName,lineNumber):
        f=open(fileName)
        lines=f.readlines()
        if lines['lineNumber'].contains("for"):
            return True
        else:
            return False

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
                insertPlanDic[fileName][endLine-1] = stringGen.genEndTimeString(startLine, endLine) 
        return insertPlanDic
    
    def calcOut(self, orig, noPar ,  par, newfile):
        f = open(newfile, 'a')
        with open(orig) as origFile, open(noPar) as noParFile,open(par) as parFile:
            for lineOrig, lineNoPar, linePar in izip(origFile, noParFile, parFile):
                print lineOrig 
                print lineNoPar.split(':')
                print linePar.split(':')
                
                if(float(lineNoPar.split(':')[2]) <= float(linePar.split(':')[2])):
                    print lineNoPar.split(':')[2]
                    f.write(lineOrig)
        f.close()