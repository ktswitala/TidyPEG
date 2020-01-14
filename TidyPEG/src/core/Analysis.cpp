
#include "useful/STDInclude.hpp"
#include "core/Analysis.hpp"

namespace PEG
{
  //void FindUnreachableChoice(const Choice& c)
  //{
    // determine the state given that the choice is matched
    // determine if the earlier choices could also be matched, given this state information
    // if they could be matched, then the latest choice cannot be matched
  //}
  void NeverFailStateGenerator::Visit( const Empty& )
  {
    // do nothing. Empty never fails on any inputs.
  }
  void NeverFailStateGenerator::Visit( const Terminal& )
  {
  }
  
  void IntermediateGenerator::Visit(const Terminal& t)
  {
    StreamStateDeclaration* ssd = topBlock->findDeclaration( t );
    if (ssd == 0)
    {
      topBlock->addAction( new CheckTerminal(t, current_offset) );

      StreamStateDeclaration* s_ssd = new StreamStateDeclaration();
      s_ssd->value = t;
      s_ssd->is_matched = true;
      s_ssd->offset = topBlock->current_offset;
      s_ssd->consumed = 1;
      
      StreamStateDeclaration* f_ssd = new StreamStateDeclaration();
      f_ssd->value = t;
      f_ssd->is_matched = false;
      f_ssd->offset = topBlock->current_offset;

      sBlock->result = true;
      sBlock->current_offset += 1; 
      sBlock->addDeclaration( s_ssd );

      fBlock->result = false;
      fBlock->addDeclaration( f_ssd );
    }
    else
    {
      if ( ssd.is_matched == true )
      {
        sBlock->result = true;
        sBlock->current_offset += 1;
      }
      else if ( ssd.is_matched == false )
      {
        fBlock->result = false;
      }
      else
      {
        throw "error Visit()";
      }
    }
  }
  void IntermediateGenerator::Visit(const NonTerminal& nt)
  {
    StreamStateDeclaration* ssd = topBlock->findDeclaration( t );
    if (ssd = 0)
    {
      topBlock->addAction( new JumpToNT(nt) );
      
      StreamStateDeclaration* s_ssd = new StreamStateDeclaration();
      s_ssd->value = nt;
      s_ssd->is_matched = true;
      s_ssd->offset = topBlock->current_offset;

      StreamStateDeclaration* f_ssd = new StreamStateDeclaration();
      f_ssd->value = nt;
      f_ssd->is_matched = false;
      f_ssd->offset = topBlock->current_offset;

      sBlock->result = true;
      sBlock->offset_known = false;
      sBlock->addDeclaration( new StreamStateDeclaration( nt, true ) );
      
      fBlock->result = false;
      fBlock->offset_known = false;
      fBlock->addDeclaration( new StreamStateDeclaration( nt, false ) );
    }
  }
  void IntermediateGenerator::Visit(const Sequence& s)
  {
    if (topBlock == 0)
    {
      topBlock = new ParserActionBlock();
    }
    for (subExpressionsType::iterator i = s.subExpressions.begin(), e = s.subExpressions.end(); i != e; i++)
    {
      (*i)->Accept( *this );
    }
  }
}
