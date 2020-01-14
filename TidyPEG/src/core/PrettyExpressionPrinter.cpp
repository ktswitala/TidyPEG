
// PrettyExpressionPrinter.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "core/PrettyExpressionPrinter.hpp"
#include "core/Expression.hpp"

using namespace std;

namespace PEG
{

PrettyExpressionPrinter::PrettyExpressionPrinter( ostream& o_in ) : o(o_in)
{
  last_print = 0;
}

void PrettyExpressionPrinter::Do(ostream& o, const Expression& g)
{
  PrettyExpressionPrinter* pep = new PrettyExpressionPrinter( o );
  g.Accept( *pep );
  delete pep;
}

void PrettyExpressionPrinter::Visit( const Empty& e)
{
  last_print = 0;
  o << "e";
}

void PrettyExpressionPrinter::Visit( const SingleTerminal& st)
{
  last_print = 0;
  o << (uint)st.value();
}

void PrettyExpressionPrinter::Visit( const RangeTerminal& rt)
{
  last_print = 0;
  o << (uint)rt.getLo() << "-" << (uint)rt.getHi();
}

void PrettyExpressionPrinter::Visit( const AnyTerminal& at)
{
  last_print = 0;
  o << ".";
}

void PrettyExpressionPrinter::Visit( const NonTerminal& nt)
{
  last_print = 0;
  o << nt.getName();
}

void PrettyExpressionPrinter::Visit( const Sequence& s )
{
  bool parens;
  if (last_print == 0)
  {
    parens = false;
  }
  else if (typeid(*last_print) == typeid(Choice) )
  {
    parens = false;
  }
  else
  {
    parens = true;
  }
  
  if (parens)
  {
    o << "(";
  }
  for (Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); i != e; i++)
  {
    last_print = &s;
    (*i)->Accept(*this);
    if ((i+1) != e)
    {
      o << " ";
    }
  }
  if (parens)
  {
    o << ")";
  }
}

void PrettyExpressionPrinter::Visit( const Choice& c )
{
  bool parens;
  if (last_print == 0)
  {
    parens = false;
  }
  else
  {
    parens = true;
  }
  if (parens)
  {
    o << "(";
  }
  for (Expression::subExpressionsType::const_iterator i = c.getExprs().begin(), e = c.getExprs().end(); i != e; i++)
  {
    last_print = &c;
    (*i)->Accept(*this);
    if ((i+1) != e)
    {
      o << " / ";
    }
  }
  if (parens)
  {
    o << ")";
  }
}

void PrettyExpressionPrinter::Visit( const Repeat& r)
{
  last_print = &r;
  r.getExprs()[0]->Accept(*this);
  o << "*";
}

void PrettyExpressionPrinter::Visit( const Not& n)
{
  last_print = &n;
  o << "!";
  n.getExprs()[0]->Accept(*this);
}

void PrettyExpressionPrinter::Visit( const Question& q)
{
  last_print = &q;
  q.getExprs()[0]->Accept(*this);
  o << "?";
}

void PrettyExpressionPrinter::Visit( const Plus& p)
{
  last_print = &p;
  p.getExprs()[0]->Accept(*this);
  o << "+";
}


}
