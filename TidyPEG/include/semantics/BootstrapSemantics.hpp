
// BootstrapSemantics.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"
#include "semantics/ParseResult.hpp"
#include "semantics/Semantics.hpp"

namespace PEG
{
  class PEGSemantics : public Semantics
  {
  public:
    PEGSemantics(ParseResult& pr_in) { } 

    void Start();
    boost::shared_ptr<Grammar> GetResult() { return newg; }
   
  private:
    boost::shared_ptr<Grammar> newg;

    void ProcessDefinitions( ParseExpr& );
    std::string format_ident( ParseExpr& );
    std::string format_literal( ParseExpr& );
    char format_char( ParseExpr& );
    void add_ident( ParseExpr& );
    Handle<Expression> generate_expr( ParseExpr& );
    Handle<Expression> generate_seq( ParseExpr& );
    Handle<Expression> generate_prefix( ParseExpr& );
    Handle<Expression> generate_range(ParseExpr&);
  
  };
}

