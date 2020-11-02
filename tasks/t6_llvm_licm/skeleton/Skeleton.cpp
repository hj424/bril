#include <string>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"

#include "llvm/Analysis/ValueTracking.h"

using namespace llvm;

namespace {
  struct SkeletonPass : public LoopPass {
    static char ID;
    SkeletonPass() : LoopPass(ID) {}

    bool changed = false;
    // This example modifies the program, but preserves CFG.
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      // Legacy analysis pass to compute loop infomation.  
      AU.addRequired<LoopInfoWrapperPass>();
      // Legacy analysis pass to compute dominator tree.
      AU.addRequired<DominatorTreeWrapperPass>();
    }

    // Find all basic blocks which are dominated by the loop header "L".
    void preOrder(DomTreeNode *N, std::vector<BasicBlock*> *loopHeaderDominatedBasicBlocks, Loop *L, DominatorTree *DT){
			BasicBlock *BB = N->getBlock();
      // if current block is being dominated, pushed to stack(vector)
			if(DT->dominates(L->getHeader(), BB)) {
				loopHeaderDominatedBasicBlocks->push_back(BB);
      }
      // recursively check the child node
			for (DomTreeNode *Child : N->children()) {
				preOrder(Child, loopHeaderDominatedBasicBlocks, L, DT);
      }
		}

    // Checks if the instruction "I" is loop invariant.
		bool isLoopInvariant(Loop *L, Instruction *I){
			if ((!isa<BinaryOperator>(*I) && !isa<CastInst>(*I) && !isa<SelectInst>(*I) && !isa<GetElementPtrInst>(*I) && !I->isShift()) || !L->hasLoopInvariantOperands(I))
				return false;
			return true;
		}

    // Checks if the instruction "I" is safe to hoist out of the loop
		bool safeToHoist(Loop *L,Instruction *I, DominatorTree *DT){
			bool IdominatesAllExitBlocks=true;
			SmallVector<BasicBlock*, 8> ExitBlocks;

      // return all the successor blocks of this loop
      L->getExitBlocks(ExitBlocks);

      // Checking if the basic block containing the instruction dominates all the exit blocks.
      for (BasicBlock *ExitBlock : ExitBlocks){
        if (!DT->dominates(I->getParent(), ExitBlock)){
          IdominatesAllExitBlocks=false;
          break;
        }
      }

  	  // isSafeToSpeculativelyExecute(I) returns true if the instruction "I" doesn't have any side effects.
			if(isSafeToSpeculativelyExecute(I) || IdominatesAllExitBlocks){
				return true;
			}
			else {
				return false;
      }
		}

    // Performs Basic LICMi
		void LICM(Loop *L, LoopInfo *LI, DominatorTree *DT, std::vector<BasicBlock*> *loopHeaderDominatedBasicBlocks){
      // return loop preheader
			BasicBlock *PreHeader = L->getLoopPreheader();
      // iterate all basic blocks dominated by the loop header
			for(BasicBlock *BB : *loopHeaderDominatedBasicBlocks){
        // if L is the inner most loop that BB lives in:
				if(LI->getLoopFor(BB) == L){
          // iterate all instructions in this BB
					for(Instruction &I:*BB){
						if(isLoopInvariant(L, &I) && safeToHoist(L, &I, DT)){
							changed=true;                             // indicator of the LICM optimization 
							I.moveBefore(PreHeader->getTerminator()); // unlink this instruction from its current BB and 
                                                        // insert it into the preheader BB
						}
					}
				}
			}
		}

    bool runOnLoop(Loop *L, LPPassManager &LPM) override {
      errs() << "LICM Optimization... \n";
      // loop analysis
	 		LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
      // get dominate tree
	 		DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	 		std::vector<BasicBlock*> loopHeaderDominatedBasicBlocks;
      // get BB which is dominated by loop "L"
	 		preOrder(DT->getRootNode(), &loopHeaderDominatedBasicBlocks, L, DT);
      // perfrom LICM optimziation
	 		LICM(L, &LI, DT, &loopHeaderDominatedBasicBlocks);
      errs() << "DONE! \n";
      // change the code if LICM optimization is performaned 
      return changed;
 		}

  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
