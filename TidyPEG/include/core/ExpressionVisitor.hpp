
// ExpressionVisitor.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once 

namespace PEG
{
  class Expression;
  class Empty;
  class SingleTerminal;
  class RangeTerminal;
  class AnyTerminal;
  class NonTerminal;
  class Sequence;
  class Choice;
  class Repeat;
  class Not;
  class Question;
  class Plus;

  class ExpressionVisitor
  {
  public:
    virtual void Visit( const Empty& ) = 0;
    virtual void Visit( const SingleTerminal& ) = 0;
    virtual void Visit( const RangeTerminal& ) = 0;
    virtual void Visit( const AnyTerminal& ) = 0;
    virtual void Visit( const NonTerminal& ) = 0;
    virtual void Visit( const Sequence& ) = 0;
    virtual void Visit( const Choice& )  = 0;
    virtual void Visit( const Repeat& ) = 0;
    virtual void Visit( const Not& ) = 0;
    virtual void Visit( const Question& ) = 0;
    virtual void Visit( const Plus& ) = 0;
    virtual ~ExpressionVisitor() { }
  };

}
