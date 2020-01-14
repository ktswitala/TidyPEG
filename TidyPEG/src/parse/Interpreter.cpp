
#include "parse/Interpreter.hpp"

using namespace std;

namespace PEG
{

static std::ostream* o = &std::cout;
  
Interpreter::Interpreter(Grammar* g_in)
{
  g = g_in;
}

bool Interpreter::BeginParse(MemoryBuffer* mb_in)
{
  parsee = mb_in->getBufferStart();
  parsee_size = mb_in->getBufferSize();
  
  current_token = parsee;
  parsee_end = parsee+parsee_size;

  fail_exprs.clear();
  fail_positions.clear();

  g->GetStart()->Accept( *this );

  if (match_success == false)
  {
    for (uint i = 0; i < fail_exprs.size(); i++)
    {
      *o << *fail_exprs[i] << endl;
    }
  }
  else
  {
    ParseRun prun;
    prun.g = g;
    prun.mb = mb_in;
    prun.trail = trail_stack;

    pr.Set( prun );
  }
  return match_success;
}

bool Interpreter::eof()
{
  assert(current_token <= parsee_end);
  if (current_token == parsee_end)
  {
    return true;
  }
  else
  {
    return false;
  } 
}

void Interpreter::SaveState(InterpretState& is)
{
  is.fail_pos = fail_exprs.size();
  is.trail_pos = trail_stack.size();
  is.stream_pos = current_token;
} 

void Interpreter::RestoreForSuccess(InterpretState& is)
{
  fail_exprs.resize( is.fail_pos );
  fail_positions.resize( is.fail_pos );
} 

void Interpreter::RestoreForFailure(InterpretState& is)
{
  current_token = is.stream_pos;
  trail_stack.resize( is.trail_pos );
} 

void Interpreter::RestoreAll(InterpretState& is)
{
  RestoreForFailure(is);
  RestoreForSuccess(is);
}

void Interpreter::Visit( const Empty& e )
{
  match_success = true;
}
void Interpreter::Visit( const SingleTerminal& st )
{
  if (!eof() && *current_token == st.value())
  {
    current_token++;
    match_success = true;
  }
  else
  {
    match_success = false;
    fail_exprs.push_back( &st );
    fail_positions.push_back( current_token );
  } 
}
void Interpreter::Visit( const RangeTerminal& rt )
{
  if(!eof() && *current_token >= rt.getLo() && *current_token <= rt.getHi())
  {
    current_token++;
    match_success = true;
  }
  else
  {
    match_success = false;
    fail_exprs.push_back( &rt );
    fail_positions.push_back( current_token );
  }
}
void Interpreter::Visit( const AnyTerminal& at )
{
  if (eof())
  {
    match_success = false;
    fail_exprs.push_back( &at );
    fail_positions.push_back( current_token );
  }
  else
  {
    current_token++;
    match_success = true;
  }
}
void Interpreter::Visit( const NonTerminal& nt )
{
  g->GetNonTerminalDefn(nt)->Accept( *this );
  if (match_success == false)
  {
    fail_exprs.push_back( &nt );
    fail_positions.push_back( current_token );
  }
}
void Interpreter::Visit( const Sequence& s )
{
  InterpretState is;
  SaveState(is);
 
  Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); 
  match_success = true;
  while (i != e && match_success == true)
  {
    RestoreForSuccess(is);
    (*i)->Accept( *this );
    i++;
  }
  if (match_success == false)
  {
    RestoreForFailure(is);

    fail_exprs.push_back( &s );
    fail_positions.push_back( current_token );
  }
}
void Interpreter::Visit( const Choice& c )
{
  InterpretState is;
  trail_stack.push_back( 666 );
  SaveState(is);
  uint the_choice = 100000;

  Expression::subExpressionsType::const_iterator i = c.getExprs().begin(), e = c.getExprs().end(); 
  match_success = false;
  while (i != e && match_success == false)
  {
    RestoreAll(is);
    (*i)->Accept( *this );
    i++;
    if (match_success == false)
    {
      the_choice++;
    }
  }
  if (match_success == false)
  {
    RestoreForFailure(is);
    trail_stack.pop_back();

    fail_exprs.push_back( &c );
    fail_positions.push_back( current_token );
  }
  else
  {
    trail_stack[is.trail_pos-1] = the_choice;
    RestoreForSuccess(is);
  }
}
void Interpreter::Visit( const Repeat& r )
{
  InterpretState ris, is;
  trail_stack.push_back( 667 );
  SaveState(ris);

  uint repeats = 0;
  match_success = true;
  while (match_success == true)
  {
    SaveState(is);
    r.getExprs()[0]->Accept( *this );
    if (match_success == true)
    {
      repeats++;
    }
  }
  RestoreAll(is);
  trail_stack[ris.trail_pos-1] = repeats;

  match_success = true;
}
void Interpreter::Visit( const Not& n)
{
  InterpretState is;
  SaveState(is);

  n.getExprs()[0]->Accept( *this );
  match_success = !match_success;

  RestoreAll(is);
  if (match_success == false)
  {
    fail_exprs.push_back( &n );
    fail_positions.push_back( current_token );
  }
}
void Interpreter::Visit( const Question& q)
{
}
void Interpreter::Visit( const Plus& p)
{
}



}
