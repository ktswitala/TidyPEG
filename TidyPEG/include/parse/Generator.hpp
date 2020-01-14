
// Generator.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "core/Expression.hpp"
#include "core/Grammar.hpp"
#include "useful/LLVMHelpers.hpp"
#include "useful/LLVMInclude.hpp"

namespace PEG
{
  using namespace llvm;

  class LLVMGenerator : public ExpressionVisitor
  {
  public:
    LLVMGenerator(Grammar&);
    ~LLVMGenerator();

    void generatePreamble();
    void generateNonTerminals();
    void generatePostamble();

    void SaveState(Value*);
    void RestoreState(Value*);
    void CheckEndOfFileCond(BasicBlock*);

    void Visit( const Empty& );
    void Visit( const SingleTerminal& );
    void Visit( const RangeTerminal& );
    void Visit( const AnyTerminal& );
    void Visit( const NonTerminal& );
    void Visit( const Sequence& );
    void Visit( const Choice& );
    void Visit( const Repeat& );
    void Visit( const Not& );
    void Visit( const Question& );
    void Visit( const Plus& );

    void ConstructTrail(vector<uint>&);
    bool runParse(const MemoryBuffer&, ParseResult&);

  private:
    Grammar& g;

    string mstr;

    Function* parse;
    Function* getTrail;

    // argument values
    Value* streamP;
    Value* streamSize;

    // entry block allocs
    StackHelper mSuccess, mFailure, mTrail;
    JumpHelper jtblSuccess, jtblFailure;

    // pointers initialized in the entry block
    Value* mStreamPosP;

    // pointers initialized in the stack unwinders
    Value* succEleP;
    Value* failEleP;

    // global blocks
    BasicBlock* entry;
    BasicBlock* startNonTerminal;
    BasicBlock* terminalTrue; 
    std::map<uint, BasicBlock*> nonTerminalBlocks;
    BasicBlock* choiceSuccess;
    BasicBlock* repeatFailure;
    BasicBlock* notSuccess;
    BasicBlock* notFailure;
    uint choiceSuccessLabel, repeatFailureLabel, notSuccessLabel, notFailureLabel;
    BasicBlock* parseError;
    BasicBlock* parseFailure;
    BasicBlock* parseSuccess;

    SwitchInst* successSwitch;
    SwitchInst* failureSwitch;
  };

}
