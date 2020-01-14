
// Grammar.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"
#include "useful/Defines.hpp"
#include "useful/Handle.hpp"
#include "core/Expression.hpp"

namespace PEG
{
  class Grammar
  {
  public:
    typedef std::map< Handle<NonTerminal>, Handle<Expression> > DefinitionsType;

    friend std::ostream& operator<<(std::ostream& o, const Grammar& g);

    void AddDefinition( Handle<NonTerminal> , Handle<Expression> );
    void SetDefinitionByName( std::string, Handle<Expression> );
    void DeclareNonTerminal( std::string );

    const DefinitionsType& GetDefinitions() const { return definitions; }

    void SetStart( Handle<NonTerminal> snt) { start = snt; } 
    Handle<NonTerminal> GetStart() const { return start; }

    const Handle<Expression> GetNonTerminalDefn(const NonTerminal& nt) const { return *ntIDToExpr.find( nt.getID() )->second; }
    const Handle<NonTerminal> GetNonTerminalByName( std::string s ) const { return *ntNameToNT.find( s )->second; }

    void Simplify();
    void FinalizeNonTerminals();
    void Finalize();
  
    static Grammar& InitializePegpeg();
  private:
    DefinitionsType definitions;
    std::map< uint, Handle<Expression>* > ntIDToExpr;
    std::map< std::string, Handle<Expression>* > ntNameToExpr;
    std::map< std::string, const Handle<NonTerminal>* > ntNameToNT; 
    Handle<NonTerminal> start;

    static Grammar pegpeg;
    static bool bootstrap_init;
  };

  std::ostream& operator<<(std::ostream& o, const Grammar& g);
}
