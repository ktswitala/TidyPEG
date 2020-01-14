
// Expression.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "core/Expression.hpp"
#include "core/PrettyExpressionPrinter.hpp"

using namespace std;
//using namespace boost;

namespace PEG
{

Handle<Expression> Expression::add(Handle<Expression> eh)
{
  subExpressions.push_back(eh);
  return eh;
}

void Expression::Promote(Handle<Expression> h)
{
  if(h->isPlusPromotable())
  {
    Plus* ne = new Plus();
    ne->add( h->subExpressions[0] );
    h.set(ne);
  }
  else if(h->isQuestionPromotable())
  {
    Question* ne = new Question();
    ne->add( h->subExpressions[0] );
    h.set(ne);
  }
  for (subExpressionsType::iterator i = h->subExpressions.begin(), e = h->subExpressions.end(); i != e; i++)
  {
    Expression::Promote( (*i) );
  }
}

void Expression::Simplify(Handle<Expression> h)
{
  if ( typeid(*h) == typeid(Question) )
  {
    Choice* e = new Choice();
    e->add( h->subExpressions[0] );
    e->add( new Empty() );
    h.set(e);
  }
  else if ( typeid(*h) == typeid(Plus) )
  {
    Sequence* e = new Sequence();
    Handle<Expression> er; 
    e->add( h->subExpressions[0] );
     er = e->add( new Repeat() );
     er->add( h->subExpressions[0] );
    h.set(e);
  } 
  for (subExpressionsType::iterator i = h->subExpressions.begin(), e = h->subExpressions.end(); i != e; i++)
  {
    Expression::Simplify( (*i) );
  }
}

void Expression::Flatten(Handle<Expression> h)
{
  subExpressionsType& ses = h->subExpressions;
  if ( typeid(*h) == typeid(Sequence) )
  {
    for (size_t i = ses.size(); i < ses.size(); i++)
    {
      if (typeid(*ses[i]) == typeid(Sequence))
      {
        Handle<Expression>& remove_me = ses[i];
        ses.erase( ses.begin() + i );
        ses.insert( ses.begin() + i, remove_me->subExpressions.begin(), remove_me->subExpressions.end() ); 
      }
    }
  }
  else if ( typeid(*h) == typeid(Choice) )
  {
    for (size_t i = ses.size(); i < ses.size(); i++)
    {
      if (typeid(*ses[i]) == typeid(Choice))
      {
        Handle<Expression> remove_me = ses[i];
        ses.erase( ses.begin() + i );
        ses.insert( ses.begin() + i, remove_me->subExpressions.begin(), remove_me->subExpressions.end() ); 
      }
    }
  }
}

bool Expression::isPlusPromotable() const
{
  if ( typeid(*this) != typeid(Sequence) )
  {
    return false;
  }
  const Sequence* s = dynamic_cast<const Sequence*>(this);
  if (s->getExprs().size() != 2)
  {
    return false;
  }
  if ( typeid(s->getExpr(1)) != typeid(Repeat) )
  {
    return false;
  }
  Repeat* r = dynamic_cast<Repeat*>( s->getExprs()[1].get() );
  if (!r)
  {
    return false;
  }
  return (s->getExpr(0) == r->getExpr(0));
}

bool Expression::isQuestionPromotable() const
{
  if ( typeid(*this) != typeid(Choice) )
  {
    return false;
  }
  const Choice* c = dynamic_cast<const Choice*>(this);
  if (c->getExprs().size() != 2)
  {
    return false;
  }
  return (typeid(c->getExpr(1)) == typeid(Empty));
}

bool Empty::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Empty))
  {
    return true;
  }
  return false;
}

bool SingleTerminal::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(SingleTerminal))
  {
    const SingleTerminal& st = dynamic_cast<const SingleTerminal&>(e);
    return (val == st.val);
  }
  return false;
}


bool RangeTerminal::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(RangeTerminal))
  {
    const RangeTerminal& rt = dynamic_cast<const RangeTerminal&>(e);
    return (hi == rt.hi && lo == rt.lo);
  }
  return false;
}

bool AnyTerminal::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(AnyTerminal))
  {
    return true;
  }
  return false;
}

bool NonTerminal::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(NonTerminal))
  {
    const NonTerminal& nt = dynamic_cast<const NonTerminal&>(e);
    return (uniqueID == nt.uniqueID);
  }
  return false;
}

bool Sequence::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Sequence))
  {
    if (getExprs().size() != e.getExprs().size())
    {
      return false;
    }
    bool accum = true;
    subExpressionsType::const_iterator i1, i2, e1, e2;
    for(i1 = getExprs().begin(), e1 = getExprs().end(), i2 = e.getExprs().begin(), e2 = e.getExprs().end(); 
         i1 != e1 && i2 != e2 && accum; i1++, i2++)
    {
      accum = accum && ( **i1 == **i2 );
    }
    return accum;
  }
  return false;
}

bool Choice::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Choice))
  {
    if (getExprs().size() != e.getExprs().size())
    {
      return false;
    }
    bool accum = true;
    subExpressionsType::const_iterator i1, i2, e1, e2;
    for(i1 = getExprs().begin(), e1 = getExprs().end(), i2 = e.getExprs().begin(), e2 = e.getExprs().end(); 
         i1 != e1 && i2 != e2; i1++, i2++)
    {
      accum = accum && ( **i1 == **i2 );
    }
    return false;
  }
  return false;
}

bool Repeat::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Repeat))
  {
    if ( getExprs().size() == 1 && e.getExprs().size() == 1 )
    {
      return ( **subExpressions.begin() == **subExpressions.end() );
    }
  }
  return false;
}

bool Not::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Not))
  {
    if ( getExprs().size() == 1 && e.getExprs().size() == 1 )
    {
      return ( **subExpressions.begin() == **subExpressions.end() );
    }
  }
  return false;
}

bool Plus::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Plus))
  {
    if ( getExprs().size() == 1 && e.getExprs().size() == 1 )
    {
      return ( **subExpressions.begin() == **subExpressions.end() );
    }
  }
  return false;
}

bool Question::operator==(const Expression& e) const
{
  if (typeid(e) == typeid(Question))
  {
    if ( getExprs().size() == 1 && e.getExprs().size() == 1 )
    {
      return ( **subExpressions.begin() == **subExpressions.end() );
    }
  }
  return false;
}

ostream& operator<<(ostream& o, const Expression& g)
{
  PrettyExpressionPrinter::Do(o, g);
  return o;
}

SingleTerminal::SingleTerminal(char c)
{
  val = c;
}

RangeTerminal::RangeTerminal(char l, char h)
{
  lo = l;
  hi = h;
}

uint NonTerminal::nextID = 1;

NonTerminal::NonTerminal(const NonTerminal& c)
{
  (*this) = c;
}

NonTerminal::NonTerminal(string n)
{
  init(n);
}

void NonTerminal::init(string n)
{
  uniqueID = NonTerminal::nextID;
  NonTerminal::nextID++;
  name = n;
}

bool NonTerminal::operator<(const NonTerminal& c) const
{
  return (this->uniqueID < c.uniqueID);
}

NonTerminal& NonTerminal::operator=(const NonTerminal& c)
{
  this->uniqueID = c.uniqueID;
  this->name = c.name;
  return (*this);
}

} // end namespace PEG
