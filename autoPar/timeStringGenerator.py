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


class TimeStringGenerator(object):
    '''
    classdocs
    '''


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

    def genEndTimeString(self, startLineNumber, endLineNumber):
        ret = 'clock_t ' + self.getEndVarName(endLineNumber) + ' = clock();\n'
        ret += 'printf("loop:' + str(startLineNumber) +':%f\\n", (double)(' + self.getEndVarName(endLineNumber) +' - '+ self.getStartVarName(startLineNumber) + ') / CLOCKS_PER_SEC);\n'
        return ret

    def genOpenMPLoop(self):
        return '#pragma omp parallel for\n'
        
    def genStartFileString(self):
        return '#include <time.h>\n'
    