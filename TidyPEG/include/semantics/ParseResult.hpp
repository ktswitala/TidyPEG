
// ParseResult.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"
#include "useful/SimplePool.hpp"
#include "useful/JITDataHelper.hpp"
#include "core/Grammar.hpp"

namespace PEG
{
  using namespace llvm;

  class ParseResult;

  // ParseExpr nodes are a bridge between the syntax and semantics of a PEG run
  // 
  class ParseExpr : public ExpressionVisitor
  {
  public:
    // Fills in the ParseExpr fields and advances the state based on the expression node passed in the parameter
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

    // The following Get functions access the ParseExpr branches (exprs)
    // They utilize RTTI to verify that the type of the Expression that this ParseExpr was created with (e) matches the template type parameter
    // The default return value is the branch node for most of these functions

    // Valid for Sequence, Choice, Repeat, and Not branch nodes
    template<class T> 
    ParseExpr& GetType( uint );

    // Valid for NonTerminal branch nodes. It performs additional checking that the name of the nonterminal is equal to the name given in the function parameter 
    ParseExpr& CheckNonTerminal( std::string, uint );

    // Same function as CheckNonTerminal, except that the return value is the branch node under the NonTerminal one. 
    // Useful because the values for the actual NonTerminal node itself, besides its name, is semantically unimportant
    ParseExpr& GetNonTerminal( std::string, uint );

    // Performs a match on the branch node to verify that it could be promoted to a Plus expression
    void GetPlus( std::vector<ParseExpr*>&, uint );

    // The Expression node that the Visit() parameter was called with for this ParseExpr node
    const Expression* e;
   
    // Converts an element from the trail stack into the proper value when it is a choice 
    uint TheChoice() { return te-100000; }

    // The trail element associated with this ParseExpr when the Expression was a Choice or Repeat
    uint te;
    // The stream positions when Visit() Started and Ended
    uint spos;
    uint epos;

    // Branch nodes of this ParseExpr 
    // This value is meaningless for Empty, Terminal, Not, Question, and Plus nodes
    // NonTerminal - exprs has a size of 1 and points to a syntax tree consistant with the nonterminal's definition
    // Sequence - exprs is equal to the number and definitions of the sequence subexpressions
    // Choice - exprs has a size of 1 and points to a syntax tree consistant with the chosen subexpression
    // Repeat - exprs has a size equal to the number of repeats during parsing, and they all have the same syntax tree form
    ParseExpr* exprs;
    uint exprcount;
 
    static ParseResult* current_result;

    // If false, the ParseExpr values do not need to be filled out
    static bool expanding;

    // If true, the ParseExpr values do need to be filled out and the construction of the ParseExpr tree continues
    static bool expand_unlimited;

    // State referenced when expanding is true and expand_unlimited is false
    // If 0, the ParseExpr values do not need to be filled out
    // If 1, the ParseExpr values should be filled except for exprs
    //   thus, the construction of the ParseExpr tree ends while the Expression tree visiting continues
    // If >1, construction continues
    static uint expand_left;
  private:
    // Simple accessor for exprs that performs an out of bounds check.
    ParseExpr& Get( uint );
  };

  // Holds the result of a full parsing run
  struct ParseRun
  {
    const Grammar* g;
    const MemoryBuffer* mb;
    vector<uint> trail;
  };

  class ParseResult
  {
  public:
    friend class ParseExpr;

    // Resets the state
    void Set(ParseRun&);

    uint getTrailElement();

    // Functions to extract the parts of the input stream that are associated with the ParseExpr nodes
    std::string Extract( ParseExpr& );
    // Extracts contiguous ParseExpr's and throws an error otherwise
    std::string Extract( ParseExpr&, ParseExpr& );
    
    ParseExpr& root() { return parseTreeRoot; }
    uint MemoryUsed() { return pool.used(); }
    uint trailSize() { return pr.trail.size(); }
  private:
    ParseRun pr;
    
    // These variables are used during ParseExpr tree construction and are modified by ParseExpr classes
    // The current stream position
    uint pos;
    // The current trail position 
    uint te_pos;

    // The result of ParseExpr tree construction for the the pr variable
    ParseExpr parseTreeRoot;
    static MemoryBlock<ParseExpr> pool;
  };


}
