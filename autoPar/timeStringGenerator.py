'''
Created on Sep 4, 2015

@author: yogev.vaknin
'''

'''

#include <time.h>

int autoPar()
{
    clock_t tic = clock();

    my_expensive_function_which_can_spawn_threads();

    clock_t toc = clock();

    printf("Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);

    return 0;
}

'''

import re
class TimeStringGenerator(object):
    '''
    classdocs
    '''

    def getLineID(self, path, startLine):
        return re.sub(r'\W+', '', path) + str(startLine)
    
    def __init__(self):
        '''
        Constructor
        '''
    def getStartVarName(self,startLineNumber):
        return 'start_time_' + str(startLineNumber)

    def getEndVarName(self,endLineNumber):
        return 'end_time_' + str(endLineNumber)

    def genStartTimeString(self, startLineNumber):
        return 'clock_t ' + self.getStartVarName(startLineNumber) + ' = clock();\n'

    def genEndTimeString(self, filePath, startLineNumber, endLineNumber):
        id = self.getLineID(filePath, startLineNumber)
        ret = 'static int was_printed_' + str(startLineNumber) +' = 0;\n'
        ret += 'clock_t ' + self.getEndVarName(endLineNumber) + ' = clock();\n'
        ret += 'if(!was_printed_' + str(startLineNumber) +') {\n printf("loop:' + str(startLineNumber) +':%f:'+id+'\\n", (double)(' + self.getEndVarName(endLineNumber) +' - '+ self.getStartVarName(startLineNumber) + ') / CLOCKS_PER_SEC);\n'
        ret += 'was_printed_' +  str(startLineNumber) +'  =1;} \n'
        return ret

    def genOpenMPLoop(self):
        return '#pragma omp parallel for\n'
        
    def genStartFileString(self):
        return '#include <time.h>\n'
    
