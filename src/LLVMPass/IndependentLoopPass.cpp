#include "llvm/Pass.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
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
	struct IndependentLoopPass : public FunctionPass {
		static char ID;
		IndependentLoopPass() : FunctionPass(ID) {}


		bool runOnFunction(Function &F) {
			errs() << "Enterting function: ";
			errs().write_escaped(F.getName()) << '\n';

			DependenceAnalysis & da = Pass::getAnalysis<DependenceAnalysis>();
			LoopInfo & LI = Pass::getAnalysis<LoopInfo>();

			//da.dump();


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
					if(LocalLoop)
					{
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
