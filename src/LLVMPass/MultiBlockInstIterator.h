//
// Created by itay on 05/09/15.
//

#ifndef PAAV15_PROJECT_MULTIBLOCKINSTITERATOR_H
#define PAAV15_PROJECT_MULTIBLOCKINSTITERATOR_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include <vector>


class MultiBlockInstIterator {

public:

    MultiBlockInstIterator(std::vector<llvm::BasicBlock *>::const_iterator first, std::vector<llvm::BasicBlock *>::const_iterator last) :
    BBfirst(first), BBlast(last)
    {
        BBit = BBfirst;
    }

    MultiBlockInstIterator(const MultiBlockInstIterator & other) : BBit(other.BBit), BIit(other.BIit), BBfirst(other.BBfirst), BBlast(other.BBlast)
    {}

    ~MultiBlockInstIterator();

    inline const llvm::Instruction & operator*()  const { return *BIit; }
    inline const llvm::Instruction * operator->() const { return &operator*(); }

    inline bool operator==(const MultiBlockInstIterator &y) const {
        return BBit == y.BBit && (BBit == BBlast || BIit == y.BIit);
    }
    inline bool operator!=(const MultiBlockInstIterator& y) const {
        return !operator==(y);
    }

    MultiBlockInstIterator& operator++() {
        ++BIit;
        advanceToNextBB();
        return *this;
    }
    inline MultiBlockInstIterator operator++(int) {
        MultiBlockInstIterator tmp = *this; ++*this; return tmp;
    }

    inline bool atEnd() const { return BBit == BBlast; }

    //inline get

private:

    inline void advanceToNextBB() {
        // The only way that the II could be broken is if it is now pointing to
        // the end() of the current BasicBlock and there are successor BBs.
        while (BIit == (*BBit)->end()) {
            ++BBit;
            if (BBit == BBlast) break;
            BIit = (*BBit)->begin();
        }
    }

    std::vector<llvm::BasicBlock *>::const_iterator BBit;
    llvm::BasicBlock::const_iterator BIit;

    std::vector<llvm::BasicBlock *>::const_iterator BBfirst;
    std::vector<llvm::BasicBlock *>::const_iterator BBlast;

};


#endif //PAAV15_PROJECT_MULTIBLOCKINSTITERATOR_H
