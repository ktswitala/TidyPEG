
// Grammar.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "core/Grammar.hpp"

using namespace std;

namespace PEG
{

Grammar Grammar::pegpeg;
bool Grammar::bootstrap_init = false;

Grammar& Grammar::InitializePegpeg()
{
  if (bootstrap_init == true)
  {
    return pegpeg;
  }

  bootstrap_init = true;
  Grammar& peg = pegpeg;

  Handle<NonTerminal> ntGrammar( new NonTerminal("grammar") );
  Handle<NonTerminal> ntDefinition( new NonTerminal("definition") );
  Handle<NonTerminal> ntExpression( new NonTerminal("expression") );
  Handle<NonTerminal> ntSequence( new NonTerminal("sequence") );
  Handle<NonTerminal> ntPrefix( new NonTerminal("prefix") );
  Handle<NonTerminal> ntSuffix( new NonTerminal("suffix") );
  Handle<NonTerminal> ntPrimary( new NonTerminal("primary") );
  Handle<NonTerminal> ntIdentifier( new NonTerminal("identifier") );
  Handle<NonTerminal> ntIdentStart( new NonTerminal("identstart") );
  Handle<NonTerminal> ntIdentCont( new NonTerminal("identcont") );
  Handle<NonTerminal> ntLiteral( new NonTerminal("literal") );
  Handle<NonTerminal> ntClass( new NonTerminal("class") );
  Handle<NonTerminal> ntRange( new NonTerminal("range") );
  Handle<NonTerminal> ntChar( new NonTerminal("char") );
  Handle<NonTerminal> ntLEFTARROW( new NonTerminal("LEFTARROW") );
  Handle<NonTerminal> ntSLASH( new NonTerminal("SLASH") );
  Handle<NonTerminal> ntAND( new NonTerminal("AND") );
  Handle<NonTerminal> ntNOT( new NonTerminal("NOT") );
  Handle<NonTerminal> ntQUESTION( new NonTerminal("QUESTION") );
  Handle<NonTerminal> ntSTAR( new NonTerminal("STAR") );
  Handle<NonTerminal> ntPLUS( new NonTerminal("PLUS") );
  Handle<NonTerminal> ntOPEN( new NonTerminal("OPEN") );
  Handle<NonTerminal> ntCLOSE( new NonTerminal("CLOSE") );
  Handle<NonTerminal> ntDOT( new NonTerminal("DOT") );
  Handle<NonTerminal> ntSpacing( new NonTerminal("spacing") );
  Handle<NonTerminal> ntComment( new NonTerminal("comment") );
  Handle<NonTerminal> ntSpace( new NonTerminal("space") );
  Handle<NonTerminal> ntEndOfLine( new NonTerminal("endofline") );
  Handle<NonTerminal> ntEndOfFile( new NonTerminal("endoffile") );

  peg.SetStart( ntGrammar );

  Handle<Expression> e1, e2, e3, e4, e5;

  e1 = new Sequence();
  e1->add( ntSpacing );
    e2 = e1->add( new Plus() );
    e2->add( ntDefinition );
  e1->add( ntEndOfFile );
  peg.AddDefinition( ntGrammar, e1 );
  
  e1 = new Sequence();
  e1->add( ntIdentifier );
  e1->add( ntLEFTARROW );
  e1->add( ntExpression );
  peg.AddDefinition( ntDefinition, e1 );

  e1 = new Sequence();
  e1->add( ntSequence );
    e2 = e1->add( new Repeat() );
      e3 = e2->add( new Sequence() );
      e3->add( ntSLASH );
      e3->add( ntSequence );
  peg.AddDefinition( ntExpression, e1 );

  e1 = new Repeat();
  e1->add( ntPrefix );
  peg.AddDefinition( ntSequence, e1 );

  e1 = new Sequence();
    e2 = e1->add( new Question() );
      e3 = e2->add( new Choice() );
      e3->add( ntAND );
      e3->add( ntNOT );
  e1->add( ntSuffix );
  peg.AddDefinition( ntPrefix, e1 );
  
  e1 = new Sequence();
  e1->add( ntPrimary );
    e2 = e1->add( new Question() );
      e3 = e2->add( new Choice() );
      e3->add( ntQUESTION );
      e3->add( ntSTAR );
      e3->add( ntPLUS );
  peg.AddDefinition( ntSuffix, e1 );

  e1 = new Choice();
    e2 = e1->add( new Sequence() );
    e2->add( ntIdentifier );
      e3 = e2->add( new Not() );
      e3->add( ntLEFTARROW );
    e2 = e1->add( new Sequence() );
    e2->add( ntOPEN );
    e2->add( ntExpression );
    e2->add( ntCLOSE );
  e1->add( ntLiteral );
  e1->add( ntClass );
  e1->add( ntDOT );
  peg.AddDefinition( ntPrimary, e1 );

  e1 = new Sequence();
  e1->add( ntIdentStart );
    e2 = e1->add( new Repeat() );
    e2->add( ntIdentCont );
  e1->add( ntSpacing );
  peg.AddDefinition( ntIdentifier, e1 );

  e1 = new Choice();
  e1->add( new RangeTerminal('a','z') );
  e1->add( new RangeTerminal('A','Z') );
  e1->add( new SingleTerminal('_') );
  peg.AddDefinition( ntIdentStart, e1 );

  e1 = new Choice();
  e1->add( ntIdentStart );
  e1->add( new RangeTerminal('0','9') );
  peg.AddDefinition( ntIdentCont, e1 );

  e1 = new Choice();
    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\'') );
      e3 = e2->add( new Repeat() );
        e4 = e3->add( new Sequence() );
          e5 = e4->add( new Not() );
          e5->add( new SingleTerminal('\'') );
        e4->add( ntChar );
    e2->add( new SingleTerminal('\'') );
    e2->add( ntSpacing );

    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\"') );
      e3 = e2->add( new Repeat() );
        e4 = e3->add( new Sequence() );
          e5 = e4->add( new Not() );
          e5->add( new SingleTerminal('\"') );
        e4->add( ntChar );
    e2->add( new SingleTerminal('\"') );
    e2->add( ntSpacing );
  peg.AddDefinition( ntLiteral, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('[') );
    e2 = e1->add( new Repeat() );
      e3 = e2->add( new Sequence() );
        e4 = e3->add( new Not() );
        e4->add( new SingleTerminal(']') );
      e3->add( ntRange );
  e1->add( new SingleTerminal(']') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntClass, e1 );

  e1 = new Choice();
    e2 = e1->add( new Sequence() );
    e2->add( ntChar );
    e2->add( new SingleTerminal('-') );
    e2->add( ntChar );
  e1->add( ntChar );
  peg.AddDefinition( ntRange, e1 );

  e1 = new Choice();
    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\\') );
      e3 = e2->add( new Choice() );
      e3->add( new SingleTerminal('n') );
      e3->add( new SingleTerminal('r') );
      e3->add( new SingleTerminal('t') );
      e3->add( new SingleTerminal('\'') );
      e3->add( new SingleTerminal('\"') );
      e3->add( new SingleTerminal('[') );
      e3->add( new SingleTerminal(']') );
      e3->add( new SingleTerminal('\\') );
    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\\') );
    e2->add( new RangeTerminal('0','2') );
    e2->add( new RangeTerminal('0','7') );
    e2->add( new RangeTerminal('0','7') );
    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\\') );
    e2->add( new RangeTerminal('0','7') );
      e3 = e2->add( new Question() );
      e3->add( new RangeTerminal('0','7') );
    e2 = e1->add( new Sequence() );
      e3 = e2->add( new Not() );
      e3->add( new SingleTerminal('\\') );
    e2->add( new AnyTerminal() );
  peg.AddDefinition( ntChar, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('<') );
  e1->add( new SingleTerminal('-') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntLEFTARROW, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('/') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntSLASH, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('&') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntAND, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('!') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntNOT, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('?') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntQUESTION, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('*') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntSTAR, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('+') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntPLUS, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('(') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntOPEN, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal(')') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntCLOSE, e1 );

  e1 = new Sequence();
  e1->add( new SingleTerminal('.') );
  e1->add( ntSpacing );
  peg.AddDefinition( ntDOT, e1 );

  e1 = new Repeat();
    e2 = e1->add( new Choice() );
    e2->add( ntSpace );
    e2->add( ntComment );
  peg.AddDefinition( ntSpacing, e1 );
 
  e1 = new Sequence();
  e1->add( new SingleTerminal('#') );
    e2 = e1->add( new Repeat() );
      e3 = e2->add( new Sequence() );
        e4 = e3->add( new Not() );
        e4->add( ntEndOfLine );
      e3->add( new AnyTerminal() );
  e1->add( ntEndOfLine );
  peg.AddDefinition( ntComment, e1 );
  
  e1 = new Choice();
  e1->add( new SingleTerminal(' ') );
  e1->add( new SingleTerminal('\t') );
  e1->add( ntEndOfLine );
  peg.AddDefinition( ntSpace, e1 );

  e1 = new Choice();
    e2 = e1->add( new Sequence() );
    e2->add( new SingleTerminal('\r') );
    e2->add( new SingleTerminal('\n') );
  e1->add( new SingleTerminal('\n') );
  e1->add( new SingleTerminal('\r') );
  peg.AddDefinition( ntEndOfLine, e1 );

  e1 = new Not();
  e1->add( new AnyTerminal() );
  peg.AddDefinition( ntEndOfFile, e1 );
  
  peg.Simplify();
  peg.Finalize();

  return peg;
}

void Grammar::AddDefinition(Handle<NonTerminal> nt, Handle<Expression> e)
{
  definitions[nt] = e;
}

void Grammar::SetDefinitionByName( string nts, Handle<Expression> e)
{
  *ntNameToExpr[ nts ] = e;
}

void Grammar::DeclareNonTerminal( string s )
{
  definitions[ Handle<NonTerminal>( new NonTerminal( s ) ) ] = Handle<Expression>();
}

void Grammar::FinalizeNonTerminals()
{
  for (DefinitionsType::iterator i = definitions.begin(), e = definitions.end(); i != e; i++)
  {
    ntNameToExpr[ i->first->getName() ] = &(i->second);
    ntNameToNT[ i->first->getName() ] = &(i->first);
  }
}

void Grammar::Finalize()
{
  FinalizeNonTerminals();
  for (DefinitionsType::iterator i = definitions.begin(), e = definitions.end(); i != e; i++)
  {
    ntIDToExpr[ i->first->getID() ] = &(i->second);
  }
}

void Grammar::Simplify()
{
  for (DefinitionsType::iterator i = definitions.begin(), e = definitions.end(); i != e; i++)
  {
    Expression::Simplify( i->second );
  }
}

ostream& operator<<(ostream& o, const Grammar& g)
{
  for(Grammar::DefinitionsType::const_iterator i = g.definitions.begin(), e = g.definitions.end(); i != e; i++)
  {
    o << *(*i).first << " <- " << *(*i).second << endl;
  }
  return o;
}

}
