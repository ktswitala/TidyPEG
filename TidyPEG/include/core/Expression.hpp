
// Expression.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"
#include "useful/Defines.hpp"
#include "useful/Handle.hpp"
#include "core/ExpressionVisitor.hpp"

namespace PEG
{
  class Expression
  {
  public:
    typedef std::vector< Handle<Expression> > subExpressionsType;

    Expression() { };
    Expression( Handle<Expression>& e ) { add( e ); }
    virtual ~Expression() { };

    // Turns Question and Plus nodes into their basic definitions
    static void Simplify(Handle<Expression>);
    // Turns expression patterns that can be turned into Question and Plus into those nodes
    static void Promote(Handle<Expression>);
    // Deletes Choice and Sequence nodes in the subExpressions of nodes of the same type
    // The subexpressions of the deleted nodes are concatenated into the subexpressions of the parents
    static void Flatten(Handle<Expression>);
    
    // Matches an expression for conversion into a Question/Plus node
    bool isPlusPromotable() const;
    bool isQuestionPromotable() const;

    // Equality is defined as follows:
    // the typeid of *this must be equal to the typeid of the parameter
    // Terminals - tests all 'token' data associated with the terminals for equality. no data = equality always true
    // NonTerminals - tests based on uniqueID
    // Sequence, Choice, Repeat, Not, Plus, Question - recursive == call on all subexpressions
    virtual bool operator==(const Expression&) const = 0;

    virtual void Accept( ExpressionVisitor& ) const = 0;

    const Expression& getExpr(uint i) const { return subExpressions[i].ref(); }
    const subExpressionsType& getExprs() const { return subExpressions; }

    // Saves some typing when constructing parsers by hand
    Handle<Expression> add(Handle<Expression>);
  protected:
    subExpressionsType subExpressions;
  private:
    Expression(const Expression&);
    Expression& operator=(const Expression&);
  };

  std::ostream& operator<<(std::ostream& o, const Expression& e);

  class Empty : public Expression
  {
  public:
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class SingleTerminal : public Expression
  {
  public:
    SingleTerminal(uchar);
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;

    uchar value() const { return val; }
  private:
    uchar val;
  };

  class RangeTerminal : public Expression
  {
  public:
    RangeTerminal(char,char);
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
   
    char getHi() const { return hi; }
    char getLo() const { return lo; }
  private:
    char hi, lo;
  };

  class AnyTerminal : public Expression
  {
  public:
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  private:
  };

  class NonTerminal : public Expression
  {
  public:
    NonTerminal(const NonTerminal&);
    NonTerminal(std::string);
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;

    // assigns a uniqueID to the NonTerminal
    void init(std::string);

    // required for storage in the STL containers
    bool operator<(const NonTerminal&) const;
    NonTerminal& operator=(const NonTerminal&);

    uint getID() const { return uniqueID; }
    std::string getName() const { return name; }
 
    static uint nextID;
  private:
    uint uniqueID;
    std::string name;
  };

  class Sequence : public Expression
  {
  public:
    Sequence() { }
    ~Sequence() { }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class Choice : public Expression
  {
  public:
    Choice() { }
    ~Choice() { } 
    Choice( Handle<Expression> e ) : Expression(e) {  }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class Not : public Expression
  {
  public:
    Not() { }
    ~Not() { }
    Not( Handle<Expression> e ) : Expression(e) {  }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class Repeat : public Expression
  {
  public:
    Repeat() { }
    ~Repeat() { }
    Repeat( Handle<Expression> e ) : Expression(e) {  }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class Optional : public Expression
  {
  public:
    Optional() { }
    ~Optional() { }
    Optional( Handle<Expression> e ) : Expression(e) {  }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };

  class Plus : public Expression
  {
  public:
    Plus() { }
    ~Plus() { }
    Plus( Handle<Expression> e ) : Expression(e) {  }
    bool operator==(const Expression&) const;
    void Accept( ExpressionVisitor& ) const;
  };
}
