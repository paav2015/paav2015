

class LineInjector(object):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''

    def inject(self, srcFileName, dstFileName, addDic):
        
        srcLines = open(srcFileName,'r').readlines()
        dest = open(dstFileName,'w')
        lineNumber = 0
        for line in srcLines:
            if (lineNumber in addDic):
                dest.write(addDic[lineNumber] + line)
            else:
                dest.write(line)
            dest.flush()
            lineNumber += 1
        dest.close()
