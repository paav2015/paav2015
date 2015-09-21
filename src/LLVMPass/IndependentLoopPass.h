//
// Created by itay on 05/09/15.
//

#ifndef PAAV15_PROJECT_INDEPENDENTLOOPPASS_H_H
#define PAAV15_PROJECT_INDEPENDENTLOOPPASS_H_H

#include <string>
#include <vector>

namespace {

    struct LoopDependencyInfo {

        LoopDependencyInfo();

        unsigned int nFirstSourceLine;
        unsigned int nSourceLineAfter;
        unsigned int nDepth;

        bool bIsLoopIndependent;
        bool bHasInductiveVariable;

        std::vector<std::string> vGEPVariables;

        std::string toSimpleString() const;
        std::string toJson() const;

        inline bool hasSourceReference() const
        {
            return (nFirstSourceLine && nSourceLineAfter);
        }

    };

    struct FunctionLoopInfo {

    public:

        std::string sFilename;
        std::string sFuncname;

        unsigned int nLastSourceLine;

        std::vector <LoopDependencyInfo> vLoops;

        std::string toSimpleString() const;
        std::string toJson() const;

    };

};


#endif //PAAV15_PROJECT_INDEPENDENTLOOPPASS_H_H
