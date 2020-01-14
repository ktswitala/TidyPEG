
// ParseResult.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "semantics/ParseResult.hpp"

using namespace std;
using namespace boost;

namespace PEG
{

ParseResult* ParseExpr::current_result = 0;
bool ParseExpr::expanding = true;
bool ParseExpr::expand_unlimited = true;
uint ParseExpr::expand_left;

MemoryBlock<ParseExpr> ParseResult::pool;

void ParseExpr::Visit( const Empty& e )
{
}

void ParseExpr::Visit( const SingleTerminal& t )
{
  current_result->pos++;  
}
void ParseExpr::Visit( const RangeTerminal& t )
{
  current_result->pos++;
}
void ParseExpr::Visit( const AnyTerminal& t ) 
{
  current_result->pos++;
}
void ParseExpr::Visit( const NonTerminal& nt ) 
{
  if (!expanding || (!expand_unlimited && !expand_left))
  {
    current_result->pr.g->GetNonTerminalDefn(nt)->Accept( *this );
    return;
  }

  spos = current_result->pos;
  e = &nt;

  if (expand_unlimited)
  {
    if (!nt.getExpand())
    {
      expanding = false;
      current_result->pr.g->GetNonTerminalDefn(nt)->Accept( *this );
    }
    else
    {
      exprs = current_result->pool.malloc( 1 );
      exprcount = 1;
      expand_left = nt.getExpandLevel();
      if (expand_left == 0)
      {
        expand_unlimited = true;
      }
      else
      {
        expand_unlimited = false;
      }
      current_result->pr.g->GetNonTerminalDefn(nt)->Accept( exprs[0] );
    }
  }
  else
  {
    expand_left--;
    if (!expand_left)
    {
      current_result->pr.g->GetNonTerminalDefn(nt)->Accept( *this );
    }
    else
    {   
      exprs = current_result->pool.malloc( 1 );
      exprcount = 1;
      current_result->pr.g->GetNonTerminalDefn(nt)->Accept( exprs[0] );
    }
  }
  epos = current_result->pos;
}

void ParseExpr::Visit( const Sequence& s )
{
  if (!expanding || (!expand_unlimited && !expand_left))
  {
    for (Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); i != e; i++)
    {
      (*i)->Accept( *this );
    }
    return;
  }

  spos = current_result->pos;
  e = &s;

  if (expand_unlimited || expand_left > 1)
  {
    if (!expand_unlimited)
    {
      expand_left--;
    }
    exprs = current_result->pool.malloc( s.getExprs().size() );
    exprcount = s.getExprs().size();
    uint exprpos = 0;
    uint save_unlimited = expand_unlimited;
    uint save_expands = expand_left;
    for (Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); i != e; i++)
    {
      (*i)->Accept( exprs[exprpos] );
      expanding = true;
      expand_unlimited = save_unlimited;
      expand_left = save_expands;
      exprpos++;					
    }
  }
  else
  {
    expand_left = 0;
    for (Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); i != e; i++)
    {
      (*i)->Accept( *this );
    }
  }
  epos = current_result->pos;
}

void ParseExpr::Visit( const Choice& c ) 
{
  uint te_in = current_result->getTrailElement();
  assert(te_in >= 100000);

  uint choice_id = te_in-100000;
  if (choice_id >= c.getExprs().size())
  {
    throw "Invalid choice in trail stack for ParseExpr::Visit(Choice&)";
  }
  Expression::subExpressionsType::const_iterator element = c.getExprs().begin() + choice_id;
  if (!expanding || (!expand_unlimited && !expand_left))
  {
    (*element)->Accept( *this );
    return;
  }

  spos = current_result->pos;
  e = &c;
  te = te_in;

  if (expand_unlimited || expand_left > 1)
  {
    if (!expand_unlimited)
    {
      expand_left--;
    }
    exprs = current_result->pool.malloc( 1 );
    exprcount = 1;
    (*element)->Accept( exprs[0] );
  }
  else
  {
    expand_left = 0;
    (*element)->Accept( *this );
  }
  epos = current_result->pos;
}

void ParseExpr::Visit( const Repeat& r )
{
  uint te_in = current_result->getTrailElement();
  assert(te_in < 100000);

  Expression::subExpressionsType::const_iterator element = r.getExprs().begin();
  if (!expanding || (!expand_unlimited && !expand_left))
  {
    for (uint i = 0; i < te_in; i++)
    {
      (*element)->Accept( *this );
    }
    return;
  }

  spos = current_result->pos;
  e = &r;
  te = te_in;
  
  if (expand_unlimited || expand_left > 1)
  {
    if (!expand_unlimited)
    {
      expand_left--;
    }
    exprs = current_result->pool.malloc( te );
    exprcount = te;
    uint save_expands = expand_left;
    bool save_unlimited = expand_unlimited;
    for (uint i = 0; i < te; i++)
    {
      (*element)->Accept( exprs[i] );
      expanding = true;
      expand_unlimited = save_unlimited;
      expand_left = save_expands;
    }
  }
  else
  { 
    expand_left = 0;
    for (uint i = 0; i < te; i++)
    {
      (*element)->Accept( *this );
    } 
  }
  epos = current_result->pos;
}

void ParseExpr::Visit( const Not& n )
{
}

void ParseExpr::Visit( const Question& q )
{
  throw string("Unsimplified grammar in ParseExpr::Visit(Question)");
}

void ParseExpr::Visit( const Plus& q )
{
  throw string("Unsimplified grammar in ParseExpr::Visit(Plus)");
}

ParseExpr& ParseExpr::Get( uint pos )
{
  if (pos >= exprcount)
  {
    throw string("Invalid position in ParseExpr::Get");
  }
  return exprs[pos];
}

template<class T> 
ParseExpr& ParseExpr::GetType( uint pos )
{
  ParseExpr& p = Get( pos );
  if (typeid(*p.e) != typeid(T))
  {
    std::cout << *p.e << std::endl;
    throw string("Invalid type expression in ParseExpr::GetType - ") + typeid(*p.e).name();
  }
  return p;
}

template ParseExpr& ParseExpr::GetType<Sequence>( uint );
template ParseExpr& ParseExpr::GetType<Choice>( uint );
template ParseExpr& ParseExpr::GetType<Repeat>( uint );
template ParseExpr& ParseExpr::GetType<Not>( uint );

ParseExpr& ParseExpr::CheckNonTerminal( string nts, uint pos )
{
  ParseExpr& nt = Get( pos );

  if (typeid(*nt.e) != typeid(NonTerminal))
  {
    throw string("Invalid nonterminal expression in ParseExpr::CheckNonTerminal ") + nts;
  }

  const NonTerminal* nte = dynamic_cast<const NonTerminal*>(nt.e);
  if (nte->getName() != nts)
  {
    throw string("Invalid nonterminal name in ParseExpr::CheckNonTerminal. Tried: ") + nts + " Actual: " + nte->getName();
  }
  
  return nt;
}

ParseExpr& ParseExpr::GetNonTerminal( string nts, uint pos )
{
  ParseExpr& nt = CheckNonTerminal( nts, pos ); 
  return nt.exprs[0];
}

void ParseExpr::GetPlus( vector<ParseExpr*>& v, uint pos )
{
  ParseExpr& plusexpr = Get( pos );

  // TODO: handle the non-promoted Expression::Plus case too.
  if (!plusexpr.e->isPlusPromotable())
  {
    throw string("Invalid plus expression in ParseExpr::GetPlus");
  }

  v.clear();
  v.push_back( &plusexpr.exprs[0] );
  ParseExpr* repexpr = &plusexpr.exprs[1];
  for (uint i = 0, e = repexpr->te; i < e; i++)
  {
    v.push_back( &repexpr->exprs[i] );
  } 

}

void ParseResult::Set(ParseRun& pr_in)
{
  pr = pr_in;
  
  pos = 0; te_pos = 0;
  ParseExpr::current_result = this;
  ParseExpr::expanding = true;
  ParseExpr::expand_unlimited = true;
  ParseResult::pool.reset();

  pr.g->GetStart().get()->Accept( parseTreeRoot );
}

uint ParseResult::getTrailElement()
{
  return pr.trail[te_pos++];
}

string ParseResult::Extract( ParseExpr& e1 )
{
  return string(pr.mb->getBufferStart()+e1.spos, pr.mb->getBufferStart()+e1.epos);
}

string ParseResult::Extract( ParseExpr& e1, ParseExpr& e2 )
{
  if (e1.epos != e2.spos)
  {
    throw string("Non-contiguous expressions in ParseResult::Extract");
  }
  return string(pr.mb->getBufferStart()+e1.spos, pr.mb->getBufferStart()+e2.epos);
}

}

