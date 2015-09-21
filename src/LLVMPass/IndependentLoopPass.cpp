#include <sstream>
#include "llvm/Pass.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/raw_ostream.h"
#include "IndependentLoopPass.h"


using namespace llvm;

#define OUTPUT_PREFIX "Independent_Loop"
#define IF_DBG if (!g_dbg) {} else
#define MY_DBG_PRINT IF_DBG errs()

static bool g_dbg = false;

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


    void printAllInstsInBlock(const BasicBlock * BB)
    {
        for (BasicBlock::const_iterator inst = BB->begin(); inst != BB->end(); ++inst)
        {
            const DebugLoc &Loc = inst->getDebugLoc();
            if (!Loc.isUnknown())
            {
                DIScope Scope(Loc.getScope());
                //DILocation * diLoc = Loc.getAsMDNode();
                //auto *Scope = cast<DIScope>(Loc.getScopeNode());
                //errs() << Scope << "\n";
                if (Scope) {
                    errs() << "File name: " << Scope.getFilename() << "\n";
                    //StringRef sName = Scope->getFilename();
                    //errs() << "File: " << sName << "\n";
                }

                //Loc.print(errs());

                errs() << "Inst line: " << Loc.getLine() << "\n";

                inst->dump();
            }
        }

    }


	struct IndependentLoopPass : public FunctionPass {

		static char ID;

		IndependentLoopPass() : FunctionPass(ID) {}


		bool runOnFunction(Function &F) {

            FunctionLoopInfo functionLoopInfo;

            ScalarEvolution & SE = getAnalysis<ScalarEvolution>();
            DependenceAnalysis & da = Pass::getAnalysis<DependenceAnalysis>();
            LoopInfo & LI = Pass::getAnalysis<LoopInfo>();

            MY_DBG_PRINT << "Enterting function: ";
            IF_DBG errs().write_escaped(F.getName()) << '\n';

#if ADVANCED_ANALYSIS
            // TODO: For finding the source line, is it enough to find the first instruction
            // with debug loc, and reduce the number of arguments from it?
            (void*) F.getArgumentList().size();
#endif

            functionLoopInfo.sFuncname = F.getName().str();

            const BasicBlock & lastBB = F.back();
            const Instruction * funcExitInst = lastBB.getTerminator();
            const DebugLoc & funcLastLoc = funcExitInst->getDebugLoc();

            // TODO: Fallback?
            if (!funcLastLoc.isUnknown()) {
                functionLoopInfo.nLastSourceLine = funcLastLoc.getLine();

                // Use it for file name as well
                DIScope Scope(funcLastLoc.getScope());
                if (Scope) {
                    functionLoopInfo.sFilename = Scope.getFilename().str();
                }
            }

            for (Function::const_iterator i = F.begin(), e = F.end(); i != e; ++i) {
                const BasicBlock &BB = *i;
                const Loop *LocalLoop = LI.getLoopFor(&BB);
                const DebugLoc &Loc = BB.begin()->getDebugLoc();
                //errs() << "Basic block " << &BB << " at line " << Loc.getLine() << ", loop " << LocalLoop << "\n";
                const Instruction *exitInst = BB.getTerminator();
                if (exitInst) {
                    const DebugLoc &exitLoc = exitInst->getDebugLoc();
                    //errs() << "Terminator (at " << exitLoc.getLine() << ") : ";
                    //exitInst->dump();
                }

            }

#if ADVANCED_ANALYSIS

            for (inst_iterator SrcI = inst_begin(F), SrcE = inst_end(F); SrcI != SrcE; ++SrcI)
            {
                // TODO: Use this code to get the "important" variable names (arrays and possibly pointers)

                Instruction *Src = &(*SrcI);
                const DebugLoc &Loc = Src->getDebugLoc();

                const Value *SrcPtr = nullptr;
                if (const LoadInst *LI = dyn_cast<LoadInst>(Src))
                    SrcPtr = LI->getPointerOperand();
                else if (const StoreInst *SI = dyn_cast<StoreInst>(Src))
                    SrcPtr = SI->getPointerOperand();
                else continue;

                const GEPOperator *SrcGEP = dyn_cast<GEPOperator>(SrcPtr);

                if (SrcGEP) {
                    errs() << "At line " << Loc.getLine() << ": ";
                    Src->dump();

                    errs() << "    SrcGEP = " << SrcGEP << "\n";
                    const SCEV *SrcPtrSCEV = SE.getSCEV(const_cast<Value *>(SrcGEP->getPointerOperand()));
                    errs() << "    SrcPtrSCEV = " << *SrcPtrSCEV << "\n";

                    const SCEV *SrcSCEV = SE.getSCEV(const_cast<Value *>(SrcPtr));
                    errs() << "    SrcSCEV = " << *SrcSCEV << "\n";

                    for (GEPOperator::const_op_iterator SrcIdx = SrcGEP->idx_begin(),
                                 SrcEnd = SrcGEP->idx_end();
                         SrcIdx != SrcEnd;
                         ++SrcIdx) {
                        const SCEV *SrcSCEV = SE.getSCEV(*SrcIdx);
                        errs() << "    SrcSCEV (in loop) = " << *SrcSCEV << "\n";
                    }
                }
            }

#endif
            //printAllInstsInBlock(&F.getEntryBlock());


            /*
            Instruction * entryInst = F.getEntryBlock().getFirstNonPHIOrDbgOrLifetime();

            if (entryInst) {
                DebugLoc loc = entryInst->getDebugLoc();
                errs() << "Function starting at " << loc.getFnDebugLoc().getLine() << "\n";
            }
             */

            for(std::vector<Loop *>::const_iterator it = LI.begin(); it != LI.end(); ++it)
            {
                LoopDependencyInfo loopDependencyInfo;
                const Loop *LocalLoop = *it;
                unsigned Depth = LocalLoop->getLoopDepth();
                loopDependencyInfo.nDepth = Depth;

                // Skip loops with depth > 1
                if (Depth > 1)
                    continue;

                const DebugLoc & localLoopStartLoc = LocalLoop->getStartLoc();

                if (!localLoopStartLoc.isUnknown()) {
                    loopDependencyInfo.nFirstSourceLine = localLoopStartLoc.getLine();
                }

                // Unless proven otherwise
                loopDependencyInfo.bIsLoopIndependent = true;

                bool hasInductiveVariable = false;
                // TODO: Find debug name, etc. of induction variable
                PHINode * phi = LocalLoop->getCanonicalInductionVariable();
                if (phi) {
                    /*
                    errs() << "Inductive variable: ";
                    phi->dump();
                    errs().flush();
                     */
                    hasInductiveVariable = true;

                    loopDependencyInfo.bHasInductiveVariable = true;
                }

                /*
                for (std::vector<BasicBlock *>::const_iterator bbIt = LocalLoop->block_begin();
                     bbIt != LocalLoop->block_end();
                     ++bbIt)
                {
                    BasicBlock * BB = *bbIt;
                    for (BasicBlock::const_iterator it = BB->begin(); it != BB->end(); ++it)
                    {

                    }
                }
                 */

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
                                        if (D->isConfused() || !D->isLoopIndependent())
                                            loopDependencyInfo.bIsLoopIndependent = false;
                                    }


                                }
                                //Instruction SrcI = *SrcInstIt;
                                //Instruction DstI = *DstInstIt;

                                // Advance, proceed to the next block if required
                                ++DstInstIt;
                                if (DstInstIt == DstBB->end())
                                {
                                    //errs() << "Jumping to next block\n";
                                    ++DstBBit;
                                    if (DstBBit != loopBlocks.end())
                                    {
                                        //errs() << "Next block found\n";
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

                    //errs() << "Exit block: " << bbExit << " for loop " << LocalLoop << "\n";

                    const Instruction * exitInst = bbExit->getTerminator();
                    if (exitInst) {
                        //errs() << "Terminator: ";
                        //exitInst->dump();
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

            //errs() << functionLoopInfo.toJson()
            errs() << functionLoopInfo.toSimpleString();

			return false;
		}


		virtual bool doInitialization(Module &M)
		{
            //g_dbg = true;
			return true;
		}

		void getAnalysisUsage(AnalysisUsage &AU) const {
			// TODO: Most are not required!
			AU.addRequired<AliasAnalysis>();
			AU.addRequired<ScalarEvolution>();

			AU.addRequired<LoopInfo>();

			AU.addRequired<MemoryDependenceAnalysis>();
			AU.addRequired<DependenceAnalysis>();
			AU.addRequiredTransitive<PostDominatorTree>();
			// We do not change anything - preserve all analysis
			AU.setPreservesAll();
		}
	};
}


char IndependentLoopPass::ID = 0;
static RegisterPass<IndependentLoopPass> X("indloop", "Independent Loop Pass", false, false);