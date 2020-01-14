
// Generator.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "parse/Generator.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

namespace PEG 
{
  
LLVMGenerator::LLVMGenerator(Grammar& g_in) : g(g_in), mSuccess("succ"), mFailure("fail"), mTrail("trail")
{
  generatePreamble();
  generateNonTerminals();
  generatePostamble();
}

LLVMGenerator::~LLVMGenerator()
{
}

void LLVMGenerator::generatePreamble()
{
  mstr += "%tokenType = type i8\n";
  mstr += "%parseType = type [ 0 x %tokenType ]\n";

  // 0-label to jump to after restore 
  // 1-stream position
  // 2-success stack position 
  // 3-trail stack position
  mstr += "%failureStackType = type { i32, [ 1024 x { i32, i32, i32, i32 } ] }\n";  

  // 0-label to jump to after restore 
  // 1-pointer to number of repeats associated with repeat success
  // 2-pointer to failure stack element associated with repeat success
  mstr += "%successStackType = type { i32, [ 1024 x { i32, i32*, { i32, i32, i32, i32 }* } ] }\n";

  // 0-choice or number of repeats 
  mstr += "%trailStackType = type { i32, [ 65536 x { i32 } ] }\n";

  mstr += "%funcType = type i1 (i32 , %parseType* )\n";
  mstr += "@mTrailStack = internal global %trailStackType zeroinitializer\n";
  mstr += "@mStreamPos = internal global i32 0\n";
  mstr += "@mHighestFail = internal global i8 0\n";
  mstr += "define %trailStackType* @getTrail()\n";
  mstr += "{\n";
  mstr += "  ret %trailStackType* @mTrailStack\n";
  mstr += "}\n";
  addParsedAssembly();

  // function creation and init
  const FunctionType* funcType = dynamic_cast<const FunctionType*>(M->getTypeByName("funcType"));
  parse = dynamic_cast<Function*>(M->getOrInsertFunction("parse", funcType));
  getTrail = M->getFunction("getTrail") ;

  // type and argument initialization
  const Type* failureStackType = M->getTypeByName("failureStackType");
  const Type* successStackType = M->getTypeByName("successStackType");
  Function::arg_iterator args = parse->arg_begin();
  streamSize = args++;
  streamP = args;

  // create blocks
  entry = BasicBlock::Create(Context, "entry", parse);
  parseSuccess = BasicBlock::Create(Context, "parseSuccess", parse);
  parseFailure = BasicBlock::Create(Context, "parseFailure", parse);
  parseError = BasicBlock::Create(Context, "parseerror", parse );

  BasicBlock* parseSuccess2 = BasicBlock::Create(Context, "parseSuccess2", parse);
  BasicBlock* parseSucceeded = BasicBlock::Create(Context, "parseSucceeded", parse);
  BasicBlock* parseFailure2 = BasicBlock::Create(Context, "parseFailure2", parse);
  BasicBlock* parseFailed = BasicBlock::Create(Context, "parseFailed", parse);

  // entry
  SetBB(entry);
  mFailure.Init( failureStackType );
  mSuccess.Init( successStackType );
  mTrail.Init( M->getGlobalVariable("mTrailStack",true) );

  mStreamPosP = M->getGlobalVariable("mStreamPos",true);

  // set the stream position to zero  
  new StoreInst( ConstantInt::get(Type::getInt32Ty(Context), 0), mStreamPosP, entry );
  
  // block: parse Success
  SetBB(parseSuccess);

  Value* succStackPos = new LoadInst( mSuccess.stackPosP, "succStackPos", parseSuccess); 
  Value* cond = new ICmpInst( *parseSuccess, ICmpInst::ICMP_EQ, 
                             succStackPos, ConstantInt::get(Type::getInt32Ty(Context), 0), "cond");
  BranchInst::Create( parseSucceeded, parseSuccess2, cond, parseSuccess );

  SetBB(parseSuccess2);
  succEleP = mSuccess.CalcElement( succStackPos );
  vector<Value*> vals = list_of( ConstantInt::get(Type::getInt32Ty(Context), 0 ) )( ConstantInt::get(Type::getInt32Ty(Context), 0 ) );
  Value* succEleLabelP = GetElementPtrInst::Create( succEleP, vals.begin(), vals.end(), "succEleLabelP", parseSuccess2 );
  Value* succEleLabel = new LoadInst( succEleLabelP, "succEleLabel", parseSuccess2 );
  successSwitch = SwitchInst::Create( succEleLabel, parseError, 0, parseSuccess2 );
  ReturnInst::Create( Context, ConstantInt::get(Type::getInt1Ty(Context), 1), parseSucceeded );

  // block: parse Failure
  SetBB(parseFailure);
  Value* failStackPos = new LoadInst( mFailure.stackPosP, "failStackPos", parseFailure); 
  cond = new ICmpInst( *parseFailure, ICmpInst::ICMP_EQ, 
                             failStackPos, ConstantInt::get(Type::getInt32Ty(Context), 0), "cond");
  BranchInst::Create( parseFailed, parseFailure2, cond, parseFailure );
  
  SetBB(parseFailure2);
  failEleP = mFailure.CalcElement( failStackPos );
  vals = list_of( ConstantInt::get(Type::getInt32Ty(Context), 0 ) )( ConstantInt::get(Type::getInt32Ty(Context), 0 ) );
  Value* failEleLabelP = GetElementPtrInst::Create( failEleP, vals.begin(), vals.end(), "succEleLabelP", parseFailure2 );
  Value* failEleLabel = new LoadInst( failEleLabelP, "succEleLabel", parseFailure2 );
  failureSwitch = SwitchInst::Create( failEleLabel, parseError, 0, parseFailure2 );
  ReturnInst::Create( Context, ConstantInt::get(Type::getInt1Ty(Context), 0), parseFailed );
  
  // block: parseError
  SetBB(parseError);
  ReturnInst::Create( Context,  ConstantInt::get(Type::getInt1Ty(Context), 0), parseError );

  // block: terminal is true
  terminalTrue = BasicBlock::Create(Context, "terminalTrue", parse);
  SetBB(terminalTrue);
  LoadInst* streampos = new LoadInst(mStreamPosP, "streampos", terminalTrue);

  vector<Value*> vs = list_of<Value*>( ConstantInt::get(Type::getInt8Ty(Context), 0) )( streampos );
  Value* tokenP = GetElementPtrInst::Create(streamP, vs.begin(), vs.end(), "tokenP", terminalTrue);
  new LoadInst(tokenP, "token", terminalTrue);
  Value* newpos = BinaryOperator::CreateAdd( streampos, ConstantInt::get(Type::getInt32Ty(Context), 1), "newpos", terminalTrue );
  new StoreInst( newpos, mStreamPosP, terminalTrue );
  BranchInst::Create( parseSuccess, terminalTrue );

  // block: choice is successful
  choiceSuccess = BasicBlock::Create( Context, "choicesuccess", parse );
  SetBB(choiceSuccess);
  choiceSuccessLabel = jtblSuccess.Add( choiceSuccess );
  mSuccess.ReleaseStack( );
  mFailure.ReleaseStack( );
  BranchInst::Create( parseSuccess, bb );

  // block: not is successful
  notSuccess = BasicBlock::Create( Context, "notsuccess", parse );
  SetBB(notSuccess);
  notSuccessLabel = jtblSuccess.Add( notSuccess );
  RestoreState( mFailure.CalcElement() );
  mFailure.ReleaseStack( );
  BranchInst::Create( parseFailure, bb );

  // block: not is a failure
  notFailure = BasicBlock::Create( Context, "notfailure", parse );
  SetBB(notFailure);
  notFailureLabel = jtblFailure.Add( notFailure );
  RestoreState( failEleP );
  mFailure.ReleaseStack( );
  BranchInst::Create( parseSuccess, bb );

  SetBB(0);
}

void LLVMGenerator::generateNonTerminals()
{
  const Grammar::DefinitionsType& defs = g.GetDefinitions();
  for (Grammar::DefinitionsType::const_iterator i = defs.begin(), e = defs.end(); i != e; i++)
  {
    nonTerminalBlocks[(*i).first->getID()] = BasicBlock::Create(Context, "nt", parse);
  }
  for (Grammar::DefinitionsType::const_iterator i = defs.begin(), e = defs.end(); i != e; i++)
  {
    SetBB(nonTerminalBlocks[(*i).first->getID()]);
    (*i).second->Accept(*this);
  }
}

void LLVMGenerator::generatePostamble()
{
  jtblSuccess.generate( successSwitch );
  jtblFailure.generate( failureSwitch );

  startNonTerminal = nonTerminalBlocks[g.GetStart()->getID()];
  BranchInst::Create( startNonTerminal, entry );

  string myerr;
  if (verifyModule(*M, ReturnStatusAction, &myerr)) {
    throw myerr;
  }
}

void LLVMGenerator::SaveState(Value* mElementP)
{
  Value* pos = new LoadInst( mStreamPosP, "SSpos", bb);
  LoadInst* stackS = new LoadInst( mSuccess.stackPosP, 0, "SSstackS", bb);
  LoadInst* stackT = new LoadInst( mTrail.stackPosP, 0, "SSstackT", bb);

  LoadInst* stackElement = new LoadInst( mElementP, "SSmElement", bb);
  Value* ss1 = InsertValueInst::Create( stackElement, pos, 1, "savestate1", bb);
  Value* ss2 = InsertValueInst::Create( ss1, stackS, 2, "savestate2", bb);
  Value* ss3 = InsertValueInst::Create( ss2, stackT, 3, "savestate3", bb);
  new StoreInst( ss3, mElementP, "newelementP", bb); 
}

void LLVMGenerator::RestoreState(Value* mElementP)
{
  LoadInst* mElement = new LoadInst( mElementP, "RSmElement", bb);
  Value* newstreampos = ExtractValueInst::Create( mElement, 1, "RSnewstreampos", bb );
  Value* newSStackpos = ExtractValueInst::Create( mElement, 2, "RSnewSstackpos", bb );
  Value* newTStackpos = ExtractValueInst::Create( mElement, 3, "RSnewTstackpos", bb );

  new StoreInst( newstreampos, mStreamPosP, bb );
  new StoreInst( newSStackpos, mSuccess.stackPosP, bb);
  new StoreInst( newTStackpos, mTrail.stackPosP, bb);
}

void LLVMGenerator::CheckEndOfFileCond(BasicBlock* nextBlock)
{
  LoadInst* streampos = new LoadInst(mStreamPosP, "EOFstreampos", bb);
  Value *cond = new ICmpInst(*bb, ICmpInst::ICMP_UGE, streampos, streamSize, "EOFcond");
  BranchInst::Create( parseFailure, nextBlock, cond, bb);
  SetBB(nextBlock);
}

void LLVMGenerator::Visit( const Empty& e)
{
  BranchInst::Create( parseSuccess, bb );
}

void LLVMGenerator::Visit( const SingleTerminal& e )
{
  BasicBlock* firstCompare = BasicBlock::Create(Context, "st", parse);
  CheckEndOfFileCond( firstCompare );
  
  LoadInst* streampos = new LoadInst(mStreamPosP, "streampos", firstCompare );
  vector<Value*> vs = list_of<Value*>( ConstantInt::get(Type::getInt8Ty(Context), 0) )( streampos );
  Value* tokenP = GetElementPtrInst::Create(streamP, vs.begin(), vs.end(), "tokenP", firstCompare);
  LoadInst* token = new LoadInst(tokenP, "token", firstCompare);

  Value *cond = new ICmpInst(*firstCompare, ICmpInst::ICMP_EQ, ConstantInt::get(Type::getInt8Ty(Context), e.value()), token, "cond");
  BranchInst::Create( terminalTrue , parseFailure, cond, firstCompare);
}

void LLVMGenerator::Visit( const RangeTerminal& e )
{
  BasicBlock* firstCompare = BasicBlock::Create(Context, "rt", parse);
  CheckEndOfFileCond( firstCompare );

  LoadInst* streampos = new LoadInst(mStreamPosP, "streampos", firstCompare);
  vector<Value*> vs = list_of<Value*>( ConstantInt::get(Type::getInt8Ty(Context), 0) )( streampos );
  Value* tokenP = GetElementPtrInst::Create(streamP, vs.begin(), vs.end(), "tokenP", firstCompare);
  LoadInst* token = new LoadInst(tokenP, "token", firstCompare);

  ICmpInst *cond = new ICmpInst(*firstCompare, ICmpInst::ICMP_UGE, token, ConstantInt::get(Type::getInt8Ty(Context), e.getLo() ), "cond");
  BasicBlock* nextCompare = BasicBlock::Create(Context, "rtn", parse);
  BranchInst::Create( nextCompare, parseFailure, cond, firstCompare);

  SetBB(nextCompare);
  cond = new ICmpInst(*nextCompare, ICmpInst::ICMP_ULE, token, ConstantInt::get(Type::getInt8Ty(Context), e.getHi() ),  "cond");
  BranchInst::Create( terminalTrue, parseFailure, cond, nextCompare);
}

void LLVMGenerator::Visit( const AnyTerminal& e )
{
  BasicBlock* firstCompare = BasicBlock::Create(Context, "at", parse);
  CheckEndOfFileCond( firstCompare );

  BranchInst::Create( terminalTrue, firstCompare );
}

void LLVMGenerator::Visit( const NonTerminal& e )
{
  BranchInst::Create( nonTerminalBlocks[e.getID()], bb );
}


void LLVMGenerator::Visit( const Sequence& s )
{
  Value* successStackElementP = mSuccess.PrepareStack();

  for (Expression::subExpressionsType::const_iterator i = s.getExprs().begin(), e = s.getExprs().end(); i != e; i++)
  {
    BasicBlock* nextBlock = BasicBlock::Create( Context, "nextSeq", parse);
    StoreToPtr( successStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), jtblSuccess.Add( nextBlock ) ) );
    (*i)->Accept(*this);

    SetBB(nextBlock);
    successStackElementP = succEleP;
  }

  mSuccess.ReleaseStack();
  BranchInst::Create( parseSuccess, bb);
}

void LLVMGenerator::Visit(const Choice& c)
{
  Value* failureStackElementP = mFailure.PrepareStack();
  Value* successStackElementP = mSuccess.PrepareStack();
  Value* trailStackElementP = mTrail.PrepareStack();

  SaveState(failureStackElementP);

  StoreToPtr( successStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), choiceSuccessLabel ) );

  uint the_choice = 100000;
  for (Expression::subExpressionsType::const_iterator i = c.getExprs().begin(), e = c.getExprs().end(); i != e; i++)
  {
    BasicBlock* nextBlock = BasicBlock::Create( Context, "nextChoice", parse);
    StoreToPtr(failureStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), jtblFailure.Add( nextBlock ) ) );
    StoreToPtr(trailStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), the_choice ) );

    (*i)->Accept(*this);

    SetBB(nextBlock);
    failureStackElementP = failEleP;
    RestoreState( failEleP );
    trailStackElementP = mTrail.CalcElement();
    the_choice++;
  }

  RestoreState( failEleP );
  mSuccess.ReleaseStack();
  mFailure.ReleaseStack();
  mTrail.ReleaseStack();
  BranchInst::Create( parseFailure, bb);
}

void LLVMGenerator::Visit(const Repeat& r)
{
  BasicBlock* repeatSuccess = BasicBlock::Create( Context, "repeatSuccess", parse );
  uint repeatSuccessLabel = jtblSuccess.Add( repeatSuccess );

  BasicBlock* repeatFailure = BasicBlock::Create( Context, "repeatFailure", parse );
  uint repeatFailureLabel = jtblFailure.Add( repeatFailure );

  Value* failureStackElementP = mFailure.PrepareStack( );
  Value* trailStackElementP = mTrail.PrepareStack( );
  SaveState(failureStackElementP);
  Value* successStackElementP = mSuccess.PrepareStack( );

  vector<Value*> vals = list_of<Value*>( ConstantInt::get(Type::getInt32Ty(Context), 0 ) )( ConstantInt::get(Type::getInt32Ty(Context), 0 ) );
  Value* trailCtP = GetElementPtrInst::Create( trailStackElementP, vals.begin(), vals.end(), "trailCtP", bb);

  StoreToPtr( failureStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), repeatFailureLabel ) );
  StoreToPtr( successStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), repeatSuccessLabel ) );
  StoreToPtr( successStackElementP, 1, trailCtP );
  StoreToPtr( successStackElementP, 2, failureStackElementP );
  StoreToPtr( trailStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), 0 ) );

  r.getExprs()[0]->Accept(*this);  

  // success condition
  SetBB(repeatSuccess);
  Value* succEle = new LoadInst( succEleP, "repsuccEle", bb );

  // load a lot of things
  failureStackElementP = ExtractValueInst::Create( succEle, 2, "repfailstackeleP", bb);
  Value* failEle = new LoadInst( failureStackElementP, "failEle", bb );
  trailCtP = ExtractValueInst::Create( succEle, 1, "reptrailCtP", bb);
  Value* trailpos = new LoadInst( mTrail.stackPosP, "trailpos", bb );
  Value* streampos = new LoadInst( mStreamPosP, "streampos", bb );

  // update the failure stack element's stream position and trail stack position with the new values
  Value* ss1 = InsertValueInst::Create( failEle, streampos, 1, "ss1", bb ); 
  Value* ss2 = InsertValueInst::Create( ss1, trailpos, 3, "ss2", bb ); 
  new StoreInst( ss2, failureStackElementP, bb );

  // increment the number of repeats in the trail element associated with this repeat
  Value* reps = new LoadInst( trailCtP, "reps", bb);
  Value* newreps = BinaryOperator::CreateAdd( reps, ConstantInt::get(Type::getInt32Ty(Context), 1), "newreps", bb );
  new StoreInst( newreps, trailCtP, bb);

  r.getExprs()[0]->Accept(*this);  

  // failure condition
  SetBB(repeatFailure);
  RestoreState( failEleP );
  mFailure.ReleaseStack( );
  BranchInst::Create( parseSuccess, bb );
}

void LLVMGenerator::Visit(const Not& n)
{
  Value* failureStackElementP = mFailure.PrepareStack( );
  SaveState( failureStackElementP );
  Value* successStackElementP = mSuccess.PrepareStack( );
  StoreToPtr( failureStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), notFailureLabel ) );
  StoreToPtr( successStackElementP, 0, ConstantInt::get(Type::getInt32Ty(Context), notSuccessLabel ) );
  n.getExprs()[0]->Accept(*this);  
}

void LLVMGenerator::Visit(const Question& q)
{
  throw "Unsimplified grammar in LLVMGenerator::Visit";
}

void LLVMGenerator::Visit(const Plus& q)
{
  throw "Unsimplified grammar in LLVMGenerator::Visit";
}

void LLVMGenerator::ConstructTrail(vector<uint>& trail)
{
  trail.clear();
  std::vector<GenericValue> noargs;
  GenericValue ts = EE->runFunction(getTrail, noargs);

  const StructType* trailStackType = dynamic_cast<const StructType*>(M->getTypeByName("trailStackType"));
  const ArrayType* trailStackArrayType = dynamic_cast<const ArrayType*>(trailStackType->getElementType(1));
  const StructType* trailStackDataType = dynamic_cast<const StructType*>(trailStackArrayType->getElementType());

  uint numElements = *jitdh->GetDataPtr<uint>(ts.PointerVal, trailStackType, 0);
  void* trailStackArray = jitdh->GetDataPtr<void>(ts.PointerVal, trailStackType, 1);
  shared_ptr<ArrayIterator<void> > ah = jitdh->GetIterator<void>( trailStackArray, trailStackArrayType );

  for (uint i = 0; i < numElements; i++)
  {
    uint result = *jitdh->GetDataPtr<uint>( ah->get(), trailStackDataType, 0 );
    trail.push_back( result );
    (*ah)++;
  }
}

bool LLVMGenerator::runParse(const MemoryBuffer& input, ParseResult& pe)
{
  std::vector<GenericValue> args(2);
  args[0].IntVal = APInt( 32, input.getBufferSize() );
  args[1].PointerVal = const_cast<char*>(input.getBufferStart());

  GenericValue gv;
  gv = EE->runFunction(parse, args);

  if (gv.IntVal.getBoolValue() == true)
  {
    vector<uint> trail;
    ConstructTrail( trail );

    ParseRun prun;
    prun.g = &g;
    prun.mb = &input;
    prun.trail = trail;

    pe.Set( prun );
    return true;
   }
  else
  {
    return false;
  }
}

}