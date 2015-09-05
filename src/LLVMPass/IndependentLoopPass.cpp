#include <sstream>
#include "llvm/Pass.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;


namespace {

    void printAllInstsInBlock(const BasicBlock * BB)
    {
        for (BasicBlock::const_iterator inst = BB->begin(); inst != BB->end(); ++inst)
        {
            const DebugLoc &Loc = inst->getDebugLoc();
            if (!Loc.isUnknown())
            {
                errs() << "Inst line: " << Loc.getLine() << "\n";
                inst->dump();
            }
        }

    }


	struct IndependentLoopPass : public FunctionPass {


		static char ID;
		IndependentLoopPass() : FunctionPass(ID) {}


		bool runOnFunction(Function &F) {
            std::stringstream ssOutput;

            ssOutput << "{\n";

			errs() << "Enterting function: ";
			errs().write_escaped(F.getName()) << '\n';

            // TODO: For finding the source line, is it enough to find the first instruction
            // with debug loc, and reduce the number of arguments from it?
            F.getArgumentList().size();

            ssOutput << "  'function' : {\n";
            ssOutput << "    { 'name' : '" << F.getName().str() << "' }\n";
            ssOutput << "  },\n";


            //printAllInstsInBlock(&F.getEntryBlock());
            /*
            Instruction * entryInst = F.getEntryBlock().getFirstNonPHIOrDbgOrLifetime();

            if (entryInst) {
                DebugLoc loc = entryInst->getDebugLoc();
                errs() << "Function starting at " << loc.getFnDebugLoc().getLine() << "\n";
            }
             */



            //errs() << "Function starting at " << F.getEntryBlock().getLandingPadInst()->getDebugLoc().getLine();

			DependenceAnalysis & da = Pass::getAnalysis<DependenceAnalysis>();
			LoopInfo & LI = Pass::getAnalysis<LoopInfo>();

            ssOutput << "  'loops' : [\n";

            for(std::vector<Loop *>::const_iterator it = LI.begin(); it != LI.end(); ++it)
            {
                const Loop *LocalLoop = *it;
                unsigned Depth = LocalLoop->getLoopDepth();
                const DebugLoc & localLoopStartLoc = LocalLoop->getStartLoc();

                ssOutput << "    {\n";
                ssOutput << "      { 'depth' : " << Depth << " },\n";

                if (!localLoopStartLoc.isUnknown())
                    ssOutput << "      { 'first_source_line' : " << localLoopStartLoc.getLine() << " },\n";

                bool hasInductiveVariable = false;
                // TODO: Find debug name, etc.
                PHINode * phi = LocalLoop->getCanonicalInductionVariable();
                if (phi) {
                    errs() << "Inductive variable: ";
                    phi->dump();
                    errs().flush();
                    hasInductiveVariable = true;
                }

                ssOutput << "      { 'inductive_var' : " << (hasInductiveVariable ? "1" : "0") << " },\n";

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

                bool hasDependencies = false;
                const std::vector<BasicBlock *> & loopBlocks = LocalLoop->getBlocks();
                std::vector<BasicBlock *>::const_iterator SrcBBit = loopBlocks.begin();
                while (SrcBBit != loopBlocks.end())
                {
                    BasicBlock * SrcBB = *SrcBBit;
                    for (BasicBlock::const_iterator SrcInstIt = SrcBB->begin(); SrcInstIt != SrcBB->end(); ++SrcInstIt)
                    {
                        if (isa<StoreInst>(*SrcInstIt) || isa<LoadInst>(*SrcInstIt))
                        {
                            BasicBlock * DstBB = SrcBB;
                            std::vector<BasicBlock *>::const_iterator DstBBit = SrcBBit;
                            BasicBlock::const_iterator DstInstIt = SrcInstIt;
                            bool hasMoreInstructions = (DstInstIt != DstBB->end());

                            errs() << "Source inst: ";
                            SrcInstIt->dump();

                            while (hasMoreInstructions)
                            {

                                if (isa<StoreInst>(*DstInstIt) || isa<LoadInst>(*DstInstIt)) {
                                    errs() << "Comparing against inst: ";
                                    DstInstIt->dump();

                                    auto D = da.depends(const_cast<Instruction *>(&(*SrcInstIt)),
                                                        const_cast<Instruction *>(&(*DstInstIt)), true);
                                    if (D) {
                                        D->dump(errs());
                                        hasDependencies = true;
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

                ssOutput << "      { 'deps' : " << (hasDependencies ? "1" : "0") << " }\n";

#if 0
                //MultiBlockInstIterator SrcI(loopBlocks.cbegin(), loopBlocks.cend());
                for (MultiBlockInstIterator SrcI(loopBlocks.cbegin(), loopBlocks.cend()); !SrcI.atEnd(); ++SrcI)
                {
                    if (isa<StoreInst>(*SrcI) || isa<LoadInst>(*SrcI)) {
                        for (MultiBlockInstIterator DstI(SrcI); !DstI.atEnd(); ++DstI)
                        {
                        //for (inst_iterator DstI = SrcI, DstE = inst_end(LocalLoop->getBlocks().end());
                           //  DstI != DstE; ++DstI) {
                            if (isa<StoreInst>(*DstI) || isa<LoadInst>(*DstI)) {
                                const DebugLoc & SrcLoc = SrcI->getDebugLoc();
                                const DebugLoc & DstLoc = DstI->getDebugLoc();
                                errs() << "Analyzing:\nSrc - line " << SrcLoc.getLine();
                                SrcI->dump();
                                errs() << "Drc - line " << DstLoc.getLine();
                                DstI->dump();
                                errs() << "da analyze - ";
                                if (auto D = da.depends(const_cast<Instruction *>(&(*SrcI)), const_cast<Instruction *>(&*DstI), true)) {
                                    D->dump(errs());
                                    for (unsigned Level = 1; Level <= D->getLevels(); Level++) {
                                        if (D->isSplitable(Level)) {
                                            errs() << "da analyze - split level = " << Level;
                                            errs() << ", iteration = " << da.getSplitIteration(*D, Level);
                                            errs() << "!\n";
                                        }
                                    }
                                }
                                else
                                    errs() << "none!\n";
                            }
                        }
                    }
                }
#endif

                ssOutput << "    },\n";
            }

            ssOutput << "  ]\n";

			//da.dump();
#if 0
            // Dump the dependence analysis - shameful copy-paste
            for (inst_iterator SrcI = inst_begin(F), SrcE = inst_end(F);
                SrcI != SrcE; ++SrcI) {
                if (isa<StoreInst>(*SrcI) || isa<LoadInst>(*SrcI)) {
                    for (inst_iterator DstI = SrcI, DstE = inst_end(F);
                         DstI != DstE; ++DstI) {
                        if (isa<StoreInst>(*DstI) || isa<LoadInst>(*DstI)) {
                            const DebugLoc & SrcLoc = SrcI->getDebugLoc();
                            const DebugLoc & DstLoc = DstI->getDebugLoc();
                            errs() << "Analyzing:\nSrc - line " << SrcLoc.getLine();
                            SrcI->dump();
                            errs() << "Drc - line " << DstLoc.getLine();
                            DstI->dump();
                            errs() << "da analyze - ";
                            if (auto D = da.depends(&*SrcI, &*DstI, true)) {
                                D->dump(errs());
                                for (unsigned Level = 1; Level <= D->getLevels(); Level++) {
                                    if (D->isSplitable(Level)) {
                                        errs() << "da analyze - split level = " << Level;
                                        errs() << ", iteration = " << da.getSplitIteration(*D, Level);
                                        errs() << "!\n";
                                    }
                                }
                            }
                            else
                                errs() << "none!\n";
                        }
                    }
                }
            }
#endif

			for (Function::const_iterator i = F.begin(), e = F.end(); i != e; ++i)
			{
				const BasicBlock & BB = *i;
				const DebugLoc & Loc = BB.begin()->getDebugLoc();
				//errs() << "Basic block (name=" << i->getName() << ", at line " << Loc.getLine() << ") has " << i->size() << " instructions.\n";
				const Loop *LocalLoop = LI.getLoopFor(&BB);
				unsigned Depth = LI.getLoopDepth(&BB);
				errs() << "Loop depth: " << Depth << "\n";
				if (Depth > 0)
				{
                    errs() << "Depth is " << Depth << " at line " << Loc.getLine() << "\n";
                    //printAllInstsInBlock(&BB);

					if(LocalLoop)
					{
                        const DebugLoc & Loc = LocalLoop->getStartLoc();
                        errs() << "Loop starts at line " << Loc.getLine() << "\n";
                        PHINode * phi = LocalLoop->getCanonicalInductionVariable();
                        if (phi) {
                            errs() << "Inductive variable: ";
                            phi->dump();
                        }
                        /*
						BasicBlock * preHeader = LocalLoop->getLoopPreheader();
						if (preHeader) {
							errs() << "Preheader: " << preHeader->begin()->getDebugLoc().getLine() << "\n";
							errs() << "Name: " << preHeader->getName() << "\n";

							for(BasicBlock::const_iterator inst = BB.begin(); inst != BB.end(); ++inst)
							{
								const DebugLoc & Loc = inst->getDebugLoc();
								errs() << *inst << "\n";
							}

							BasicBlock * loopParent = preHeader->getPrevNode();
							if (loopParent)
							{
								//errs() << "Prev: " << loopParent->begin()->getDebugLoc().getLine() << "\n";
							}

						}
                        BasicBlock * header = LocalLoop->getHeader();
                        if (header) {
                            errs() << "Header: " << header->begin()->getDebugLoc().getLine() << "\n";
                            errs() << "Name: " << header->getName() << "\n";
                            printAllInstsInBlock(header);
                        }
                        PHINode * phi = LocalLoop->getCanonicalInductionVariable();
                        if (phi) {
                            errs() << "Inductive variable: ";
                            phi->dump();
                        }
                         */
					}

				}

			}

			const BasicBlock & BB = F.getEntryBlock();
			const Instruction *I = BB.begin();
			const DebugLoc & Loc = I->getDebugLoc();
			unsigned Line = Loc.getLine();

			//StringRef File = Loc->getFilename();
			//StringRef Dir = Loc->getDirectory();
			errs() << "Line: " << Line << "\n";

			// Use LLVM's Strongly Connected Components (SCCs) iterator to produce
			// a reverse topological sort of SCCs.
			//outs() << "SCCs for " << F.getName() << " in post-order:\n";
			for (scc_iterator<Function *> sccI = scc_begin(&F), sccIE = scc_end(&F); sccI != sccIE; ++sccI) {
				// Obtain the vector of BBs in this SCC and print it out.
				const std::vector<BasicBlock *> &SCCBBs = *sccI;
				const BasicBlock * firstBlock = *(SCCBBs.begin());
				const Instruction *I = firstBlock->begin();
				const DebugLoc & Loc = I->getDebugLoc();
				unsigned Line = Loc.getLine();

				//outs() << "  Line: " << Line << " SCC: ";
				if (sccI.hasLoop())
				{
					//outs() << " LOOP!!! ";
				}

				for (std::vector<BasicBlock *>::const_iterator BBI = SCCBBs.begin(),
							 BBIE = SCCBBs.end(); BBI != BBIE; ++BBI)
				{
					//outs() << (*BBI)->getName() << "  ";
				}
				//outs() << "\n";
			}

            ssOutput << "}\n";

            std::string sOutput = ssOutput.str();
            errs() << sOutput;

			return false;
		}


		virtual bool doInitialization(Module &M)
		{
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
