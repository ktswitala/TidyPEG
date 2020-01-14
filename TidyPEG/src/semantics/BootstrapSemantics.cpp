
// BootstrapSemantics.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "semantics/BootstrapSemantics.hpp"

using namespace std;
using namespace boost;

namespace PEG
{

void PEGSemantics::Start()
{
  newg.reset( new Grammar() );
  ParseExpr& pe = presult.root().GetType<Sequence>(0);
  ProcessDefinitions( pe );
}

void PEGSemantics::ProcessDefinitions( ParseExpr& defns )
{
  vector<ParseExpr*> vdef;
  defns.GetPlus( vdef, 1 );

  for (uint i = 0; i < vdef.size(); i++)
  {
    ParseExpr& next_def = vdef[i]->GetType<Sequence>(0);
    ParseExpr& next_ident = next_def.GetNonTerminal("identifier", 0 );
    add_ident( next_ident );
  }

  newg->FinalizeNonTerminals();
  for (uint i = 0; i < vdef.size(); i++)
  {
    ParseExpr& next_def = vdef[i]->GetType<Sequence>(0);
    ParseExpr& next_ident = next_def.GetNonTerminal("identifier", 0 );
    ParseExpr& next_expr = next_def.GetNonTerminal("expression", 2 );   
    string next_ident_str = format_ident( next_ident );
    newg->SetDefinitionByName( next_ident_str, generate_expr( next_expr ) );
  }
}

string PEGSemantics::format_ident( ParseExpr& ident )
{
  string result = presult.Extract( ident.CheckNonTerminal("identstart", 0), ident.GetType<Repeat>(1) );
  return result;
}
string PEGSemantics::format_literal( ParseExpr& lit )
{
  ParseExpr& litrep = lit.GetType<Sequence>(0).GetType<Repeat>( 1 );
  string result;
  for (uint i = 0; i < litrep.exprcount; i++)
  {
    result += format_char( litrep.GetType<Sequence>(i).GetNonTerminal("char", 1) );
  }
  return result;
}
char PEGSemantics::format_char( ParseExpr& tc )
{
  char result = 0;
  string cs = presult.Extract( tc );
  if (tc.TheChoice() == 0)
  {
    if (cs[1] == 'n')
    {
      result = '\n';
    }
    else if (cs[1] == 'r')
    { 
      result = '\r'; 
    }
    else if (cs[1] == 't')
    {
      result = '\t'; 
    }
    else 
    {
      result = cs[1];
    }
  }
  else if (tc.TheChoice() == 1)
  {
    result = strtol(cs.c_str()+1, 0, 8); 
  }
  else if (tc.TheChoice() == 2)
  {
    result = strtol(cs.c_str()+1, 0, 8);
  }
  else if (tc.TheChoice() == 3)
  {
    result = cs[0];
  }
  return result;
}
void PEGSemantics::add_ident( ParseExpr& ident )
{
  string newname = format_ident( ident );
  newg->DeclareNonTerminal( newname );
}
Handle<Expression> PEGSemantics::generate_expr( ParseExpr& expr )
{
  Handle<Expression> ep;
  ParseExpr& seq = expr.GetNonTerminal( "sequence", 0 );
  ParseExpr& seqs = expr.GetType<Repeat>( 1 );

  if (seqs.exprcount != 0)
  {
    ep.reset( new Choice() );
    ep->add( generate_seq( seq ) );
      
    for (uint i = 0; i < seqs.exprcount; i++)
    {
      ep->add( generate_seq( seqs.GetType<Sequence>(i).GetNonTerminal( "sequence", 1) ) );
    }
  }
  else
  {
    ep = generate_seq( seq );
  }
  return ep;
}
Handle<Expression> PEGSemantics::generate_seq( ParseExpr& seq )
{
  Handle<Expression> ep;
  assert(seq.exprcount != 0);
  if (seq.exprcount == 1)
  {
    ep = generate_prefix( seq.GetNonTerminal( "prefix", 0 ) );
  }
  else
  {
    ep.reset( new Sequence() );
    for (unsigned int i = 0; i < seq.exprcount; i++)
    {
      ep->add( generate_prefix( seq.GetNonTerminal( "prefix", i ) ) );
    } 
  }
  return ep;
}
Handle<Expression> PEGSemantics::generate_prefix( ParseExpr& pfix )
{
  ParseExpr& sfix = pfix.GetNonTerminal( "suffix", 1 );
  ParseExpr& prim = sfix.GetNonTerminal( "primary", 0 );

  Handle<Expression> ep;
  if (prim.TheChoice() == 0)
  {
    ParseExpr& nid = prim.GetType<Sequence>(0).GetNonTerminal( "identifier", 0 );
    string the_id = format_ident( nid );
    ep = newg->GetNonTerminalByName( the_id );
  }
  else if (prim.TheChoice() == 1)
  {
    ep = generate_expr( prim.GetType<Sequence>(0).GetNonTerminal("expression", 1) );
  }
  else if (prim.TheChoice() == 2)
  {
    ParseExpr& lit = prim.GetNonTerminal( "literal", 0 );
    string cs = format_literal(lit);
      
    ep.reset( new Sequence() );
    for(unsigned int i = 0; i < cs.length(); i++)
    {
      ep->add( Handle<SingleTerminal>(new SingleTerminal(cs[i])) );
    }
  }
  else if (prim.TheChoice() == 3)
  {
    ParseExpr& classn = prim.GetNonTerminal("class", 0 );
    ParseExpr& rngs = classn.GetType<Repeat>(1);
    if (rngs.exprcount == 1)
    {
      ParseExpr& rng = rngs.GetType<Sequence>(0).GetNonTerminal("range", 1 );
      ep = generate_range( rng );
    }
    else
    {
      ep.reset( new Choice() );
      for (uint i = 0; i < rngs.exprcount; i++)
      {
        ParseExpr& rng = rngs.GetType<Sequence>(i).GetNonTerminal("range", 1 );
        ep->add( generate_range( rng ) );
      }
    }
  }
  else if (prim.TheChoice() == 4)
  {
    prim.CheckNonTerminal("DOT", 0 );
    ep.reset(new AnyTerminal());
  }

  if ( sfix.GetType<Choice>(1).TheChoice() == 0 )
  {
    ParseExpr& sn = sfix.GetType<Choice>(1).GetType<Choice>(0);
    if (sn.TheChoice() == 0)
    {
      ep.reset( new Question(ep) );
    }
    else if (sn.TheChoice() == 1)
    {
      ep.reset( new Repeat(ep) );
    }
    else if (sn.TheChoice() == 2)
    {
      ep.reset( new Plus(ep) );
    } 
  }

  if (pfix.GetType<Choice>(0).TheChoice() == 0 )
  {
    ParseExpr& andnot = pfix.GetType<Choice>(0).GetType<Choice>(0);
    if (andnot.TheChoice() == 0)
    {
      ep.reset( new Not( Handle<Expression>(new Not(ep)) ) );
    }
    else if (andnot.TheChoice() == 1)
    {
      ep.reset( new Not( ep ) );
    }
  }
  return ep;
}
Handle<Expression> PEGSemantics::generate_range(ParseExpr& rng)
{
  Handle<Expression> ep;

  if (rng.TheChoice() == 0)
  {
    ParseExpr& charlo = rng.GetType<Sequence>(0).GetNonTerminal("char", 0 );
    ParseExpr& charhi = rng.GetType<Sequence>(0).GetNonTerminal("char", 2 );
    ep.reset( new RangeTerminal( format_char(charlo), format_char(charhi) ) );
  }
  else if (rng.TheChoice() == 1)
  {
    ParseExpr& charn = rng.GetNonTerminal("char", 0 );
    ep.reset( new SingleTerminal( format_char(charn) ) );
  }
  return ep;
}

}

