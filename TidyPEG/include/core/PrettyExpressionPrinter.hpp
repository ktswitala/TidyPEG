
// PrettyExpressionPrinter.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"
#include "useful/Defines.hpp"
#include "core/ExpressionVisitor.hpp"

namespace PEG
{
  class PrettyExpressionPrinter : public ExpressionVisitor
  {
  public:
    PrettyExpressionPrinter( std::ostream& );

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

    static void Do(std::ostream&, const Expression& );
  private:
    const Expression* last_print;
    std::ostream& o;
  };
}
