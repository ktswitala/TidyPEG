
// Interpreter.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "core/Expression.hpp"
#include "core/Grammar.hpp"
#include "useful/LLVMInclude.hpp"

namespace PEG
{
  
struct InterpretState
{
  uint fail_pos, trail_pos;
  const char* stream_pos;
};

class Interpreter : public ExpressionVisitor
{
  Interpreter(Grammar*);
  bool BeginParse(MemoryBuffer*);
  bool eof();

  void SaveState(InterpretState&);
  void RestoreForSuccess(InterpretState&);
  void RestoreForFailure(InterpretState&);
  void RestoreAll(InterpretState&);

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

  Grammar* g;
  const char *parsee, *parsee_end, *current_token;
  uint parsee_size;
  bool match_success;
  vector<const Expression*> fail_exprs;
  vector<const char*> fail_positions;
  vector<uint> trail_stack;
  
  ParseResult pr;
};

}