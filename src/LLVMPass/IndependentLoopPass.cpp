#include <sstream>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "IndependentLoopPass.h"


using namespace llvm;

#define OUTPUT_PREFIX "Independent_Loop"
#define IF_DBG if (!g_dbg) {} else
#define MY_DBG_PRINT IF_DBG errs()

static cl::opt<bool> g_dbg("indloop-dbg", cl::init(false), cl::Hidden, cl::ZeroOrMore, cl::desc("Enable debug prints."));
static cl::opt<bool> g_printJson("indloop-json", cl::init(false), cl::Hidden, cl::ZeroOrMore, cl::desc("Use JSON as output format."));

namespace {

    LoopDependencyInfo::LoopDependencyInfo() : nFirstSourceLine(0), nSourceLineAfter(0), nDepth(0),
                                               bIsLoopIndependent(false), bHasInductiveVariable(false)
    {}

    std::string LoopDependencyInfo::toJson() const {
        std::stringstream ssOutput;

        ssOutput << "    {\n";
        ssOutput << "      { 'depth' : " << nDepth << " },\n";
        ssOutput << "      { 'first_source_line' : " << nFirstSourceLine << " },\n";
        ssOutput << "      { 'last_source_line' : " << nSourceLineAfter << " }\n";
        ssOutput << "      { 'inductive_var' : " << (bHasInductiveVariable ? "1" : "0") << " },\n";
        ssOutput << "      { 'independent' : " << (bIsLoopIndependent ? "1" : "0") << " }\n";
        ssOutput << "      { 'GEP_vars' : [";
        for (std::vector<std::string>::const_iterator it = vGEPVariables.begin(); it != vGEPVariables.end(); ++it) {
            ssOutput << "'" << *it << "', ";
        }
        // Remove the last ','
        ssOutput.seekp(-1, ssOutput.cur);
        ssOutput << "]\n";
        ssOutput << "    }";

        return ssOutput.str();
    }

    std::string LoopDependencyInfo::toSimpleString() const {
        std::stringstream ssOutput;

        ssOutput << nFirstSourceLine << ":" << nSourceLineAfter;

        return ssOutput.str();
    }

    std::string FunctionLoopInfo::toJson() const {
        std::stringstream ssOutput;

        ssOutput << "{\n";
        ssOutput << "  'file' : {\n";
        ssOutput << "    { 'name' : '" << sFilename << "' }\n";
        ssOutput << "  },\n";
        ssOutput << "  'function' : {\n";
        ssOutput << "    { 'name' : '" << sFuncname << "' },\n";
        ssOutput << "    { 'last_source_line' : '" << nLastSourceLine << "' }\n";
        ssOutput << "  }";

        if (vLoops.size() > 0)
        {
            ssOutput << ",\n";
            ssOutput << "  'loops' : [\n";

            for(std::vector<LoopDependencyInfo>::const_iterator it = vLoops.begin(); it != vLoops.end(); ++it)
            {
                ssOutput << it->toJson();
                ssOutput << ",\n";
            }
            // Remove the last ",\n"
            ssOutput.seekp(-2, ssOutput.cur);
            ssOutput << "\n";

            ssOutput << "  ]\n";
        }
        else
        {
            ssOutput << "\n";
        }

        ssOutput << "}\n";

        return ssOutput.str();
    }

    std::string FunctionLoopInfo::toSimpleString() const
    {
        std::stringstream ssOutput;

        for(std::vector<LoopDependencyInfo>::const_iterator it = vLoops.begin(); it != vLoops.end(); ++it)
        {
            const LoopDependencyInfo & curLoop = *it;

            // Only print loops which are independent, and has source information
            if (curLoop.hasSourceReference() && curLoop.bIsLoopIndependent)
            {
                ssOutput << OUTPUT_PREFIX << ":";
                ssOutput << sFilename << ":";
                ssOutput << curLoop.toSimpleString();
                ssOutput << "\n";
            }
        }

        return ssOutput.str();
    }


	struct IndependentLoopPass : public FunctionPass {

		static char ID;

		IndependentLoopPass() : FunctionPass(ID) {}


		bool runOnFunction(Function &F) {

            FunctionLoopInfo functionLoopInfo;

            DependenceAnalysis & da = Pass::getAnalysis<DependenceAnalysis>();
            LoopInfo & LI = Pass::getAnalysis<LoopInfo>();

            MY_DBG_PRINT << "Enterting function: ";
            IF_DBG errs().write_escaped(F.getName()) << '\n';

            functionLoopInfo.sFuncname = F.getName().str();

            const BasicBlock & lastBB = F.back();
            const Instruction * funcExitInst = lastBB.getTerminator();
            const DebugLoc & funcLastLoc = funcExitInst->getDebugLoc();

            if (!funcLastLoc.isUnknown()) {
                functionLoopInfo.nLastSourceLine = funcLastLoc.getLine();

                // Use it for file name as well
                DIScope Scope(funcLastLoc.getScope());
                if (Scope) {
                    functionLoopInfo.sFilename = Scope.getFilename().str();
                }
            }

            for(std::vector<Loop *>::const_iterator it = LI.begin(); it != LI.end(); ++it)
            {
                LoopDependencyInfo loopDependencyInfo;
                const Loop *LocalLoop = *it;
                unsigned Depth = LocalLoop->getLoopDepth();
                loopDependencyInfo.nDepth = Depth;

                const DebugLoc & localLoopStartLoc = LocalLoop->getStartLoc();
                if (!localLoopStartLoc.isUnknown()) {
                    loopDependencyInfo.nFirstSourceLine = localLoopStartLoc.getLine();
                }

                MY_DBG_PRINT << "Enterting loop starting in line " << loopDependencyInfo.nFirstSourceLine << "\n";

                // Skip loops with depth > 1
                if (Depth > 1) {
                    MY_DBG_PRINT << "Loop depth is " << loopDependencyInfo.nDepth << " - skip!\n";
                    continue;
                }

                // Unless proven otherwise
                loopDependencyInfo.bIsLoopIndependent = true;

                PHINode * phi = LocalLoop->getCanonicalInductionVariable();
                if (phi) {

                    MY_DBG_PRINT << "Inductive variable: ";
                    IF_DBG phi->dump();
                    loopDependencyInfo.bHasInductiveVariable = true;
                }

                const std::vector<BasicBlock *> & loopBlocks = LocalLoop->getBlocks();
                std::vector<BasicBlock *>::const_iterator SrcBBit = loopBlocks.begin();
                while (SrcBBit != loopBlocks.end() && loopDependencyInfo.bIsLoopIndependent)
                {
                    BasicBlock * SrcBB = *SrcBBit;
                    for (BasicBlock::const_iterator SrcInstIt = SrcBB->begin();
                         SrcInstIt != SrcBB->end() && loopDependencyInfo.bIsLoopIndependent;
                         ++SrcInstIt)
                    {
                        if (isa<StoreInst>(*SrcInstIt) || isa<LoadInst>(*SrcInstIt))
                        {
                            BasicBlock * DstBB = SrcBB;
                            std::vector<BasicBlock *>::const_iterator DstBBit = SrcBBit;
                            BasicBlock::const_iterator DstInstIt = SrcInstIt;
                            bool hasMoreInstructions = (DstInstIt != DstBB->end());

                            MY_DBG_PRINT << "Source inst: ";
                            IF_DBG SrcInstIt->dump();

                            while (hasMoreInstructions && loopDependencyInfo.bIsLoopIndependent)
                            {

                                if (isa<StoreInst>(*DstInstIt) || isa<LoadInst>(*DstInstIt)) {
                                    MY_DBG_PRINT << "Comparing against inst: ";
                                    IF_DBG DstInstIt->dump();

                                    auto D = da.depends(const_cast<Instruction *>(&(*SrcInstIt)),
                                                        const_cast<Instruction *>(&(*DstInstIt)), true);
                                    if (D) {
                                        MY_DBG_PRINT << "Dependencies in loop starting at " << loopDependencyInfo.nFirstSourceLine << "\n";
                                        IF_DBG D->dump(errs());

                                        // In doubt, assume dependence
                                        if (!D->isInput() && (D->isConfused() || !D->isLoopIndependent()))
                                            loopDependencyInfo.bIsLoopIndependent = false;
                                    }


                                }

                                // Advance, proceed to the next block if required
                                ++DstInstIt;
                                if (DstInstIt == DstBB->end())
                                {
                                    ++DstBBit;
                                    if (DstBBit != loopBlocks.end())
                                    {
                                        DstBB = *DstBBit;
                                        DstInstIt = DstBB->begin();
                                    }
                                    else
                                    {
                                        hasMoreInstructions = false;
                                    }
                                }
                            }


                        }
                    }

                    ++SrcBBit;
                }

                // Try to understand where is the first block after this loop ends
                // This is unstable, as it relies on understanding whether the source line
                // of the basic block exiting the loop comes after the loop itself, will
                // fail when the next instruction comes in the same line, and maybe on other
                // cases as well
                // TODO: Also, we are still not sure about the case where there are multiple
                // exit points
                BasicBlock * bbExit = LocalLoop->getExitBlock();
                while (bbExit &&
                        loopDependencyInfo.nFirstSourceLine != 0 &&
                        !loopDependencyInfo.nSourceLineAfter) {

                    const Instruction * exitInst = bbExit->getTerminator();
                    if (exitInst) {
                        DebugLoc exitInstLoc = exitInst->getDebugLoc();
                        unsigned int exitLine = exitInstLoc.getLine();
                        if (exitLine != loopDependencyInfo.nFirstSourceLine)
                            loopDependencyInfo.nSourceLineAfter = exitLine;
                    }

                    if (!loopDependencyInfo.nSourceLineAfter)
                    {
                        bbExit = bbExit->getNextNode();
                    }

                }

                functionLoopInfo.vLoops.push_back(loopDependencyInfo);


            }

            if (g_printJson) {
                errs() << functionLoopInfo.toJson();
            } else {
                errs() << functionLoopInfo.toSimpleString();
            }

			return false;
		}


		virtual bool doInitialization(Module &M)
		{
			return true;
		}

		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfo>();
			AU.addRequired<DependenceAnalysis>();
			// We do not change anything - preserve all analysis
			AU.setPreservesAll();
		}
	};
}


char IndependentLoopPass::ID = 0;
static RegisterPass<IndependentLoopPass> X("indloop", "Independent Loop Pass", false, false);