
#pragma once

namespace PEG
{
  class NeverFailStateGenerator : ExpressionVisitor
  {
  public:
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
  private:
    uint current_position;
    vector<StreamStateDeclaration> state;
  };
  
  class ReferencePosition
  {
    map< pair<const Expression*, uint>, StreamStateDeclaration> ssds;
  };
  class StreamStateDeclaration
  {
    const Expression* value;
    bool is_matched;
    uint offset;
    bool know_consumed;
    uint consumed;
  };
  class ParserAction;
  {
    virtual ParserAction() = 0;
  };
  class CheckTerminal : public ParserAction
  {
    uint position;
    Terminal* value;
  };
  class PushTrail : public ParserAction
  {
    uint value;
  };
  class PositionIncrement : public ParserAction
  {
    uint amount;
  };
  class JumpToBlock : public ParserAction
  {
    ParserActionBlock* repeat;
  };
  class JumpToNT : public ParserAction
  {
    NonTerminal* nt;
  };
  class ParserActionBlock
  {
    ReferencePosition r;
    vector<ParserAction> pas;

    bool result;
    uint current_offset;
    bool offset_known;
  };
}
