
#include "useful/LLVMHelpers.cpp"

namespace PEG
{
  using namespace llvm;

StackHelper::StackHelper(string n)
{
  name = n;
}

void StackHelper::Init(const Type* t)
{
  // Allocate memory for the stack based on the type parameter
  stackP = new AllocaInst( t, name + "initStackP", bb );
  
  // Precalculate the address of the stack position
  vector<Value*> evs = list_of<Value*>( ConstantInt::get(Type::getInt32Ty(Context), 0) )( ConstantInt::get(Type::getInt32Ty(Context), 0) );
  stackPosP = GetElementPtrInst::Create( stackP, evs.begin(), evs.end(), name + "initstackPosP", bb );

  // Set the stack position to 0
  new StoreInst( ConstantInt::get(Type::getInt32Ty(Context), 0), stackPosP, bb );
}
void StackHelper::Init(Value* v)
{
  // Use an already allocated stack
  stackP = v;
  
  // Calculate and store the address of the stack position
  vector<Value*> evs = list_of<Value*>( ConstantInt::get(Type::getInt32Ty(Context), 0) )( ConstantInt::get(Type::getInt32Ty(Context), 0) );
  stackPosP = GetElementPtrInst::Create( stackP, evs.begin(), evs.end(), name + "initstackPosP", bb );

  // Set the stack position to 0
  new StoreInst( ConstantInt::get(Type::getInt32Ty(Context), 0), stackPosP, bb );
}

// Loads the top stack element based on the current stack position
Value* StackHelper::CalcElement()
{
  LoadInst* stackpos = new LoadInst(stackPosP, name + "CE0stackpos", bb);
  return CalcElement( stackpos );
}

// Calculates stack element address based on the position argument, but subtracts 1 (because the top of stack position really points to the bottom empty element)
Value* StackHelper::CalcElement(Value* stackpos)
{
  Value* succElePos = BinaryOperator::CreateSub( stackpos, ConstantInt::get(Type::getInt32Ty(Context), 1), "CE1succElePos", bb );
  vector<Value*> vs = list_of<Value*>(ConstantInt::get(Type::getInt32Ty(Context), 0) )( ConstantInt::get(Type::getInt32Ty(Context), 1) )( succElePos );
  return GetElementPtrInst::Create( stackP, vs.begin(), vs.end(), name + "CE1stackeleP", bb);
}

// Calculates stack element address based on the position argument
Value* StackHelper::CalcElementById(Value* idpos)
{
  vector<Value*> vs = list_of<Value*>(ConstantInt::get(Type::getInt32Ty(Context), 0) )( ConstantInt::get(Type::getInt32Ty(Context), 1) )( idpos );
  return GetElementPtrInst::Create( stackP, vs.begin(), vs.end(), name + "CIstackeleP", bb);
}
Value* StackHelper::PrepareStack()
{
  // Add one to the stack position
  LoadInst* stackPos = new LoadInst(stackPosP, name + "PSstackpos", bb);
  Value* newpos = BinaryOperator::CreateAdd( stackPos, ConstantInt::get(Type::getInt32Ty(Context), 1), name + "PSnewpos", bb);
  new StoreInst( newpos, stackPosP, bb);

  // Return the element before the increment
  return CalcElementById( stackPos );
}
void StackHelper::ReleaseStack()
{
  // Subtract one from the stack position
  LoadInst* stackPos = new LoadInst(stackPosP, name + "RSstackpos", bb);
  Value* newpos = BinaryOperator::CreateSub( stackPos, ConstantInt::get(Type::getInt32Ty(Context), 1), name + "RSnewpos", bb);
  new StoreInst( newpos, stackPosP, bb);
}

// Fills the jump table
void JumpHelper::generate(SwitchInst* si)
{
  for (uint i = 1; i < jumps.size(); i++)
  {
    si->addCase( ConstantInt::get( Type::getInt32Ty(Context), i) , jumps[i] );
  }
}

uint JumpHelper::Add(BasicBlock* jbb)
{
  jumps.push_back( jbb );
  return jumps.size()-1;
}

BasicBlock* bb = 0;
LLVMContext& Context = getGlobalContext();

void SetBB(BasicBlock* newbb)
{
  bb = newbb;
}

  M = new Module("PEG", getGlobalContext());
  if (!M)
  {
    throw "Module construction failed in LLVMGenerator::LLVMGenerator()";
  }
  OMP = new ExistingModuleProvider(M);
  EE = EngineBuilder(OMP).create();
  jitdh = new JITDataHelper(EE);
  
void LLVMGenerator::StoreToPtr(Value* ptr, uint offset, Value* v)
{
  vector<Value*> vals = list_of( ConstantInt::get(Type::getInt32Ty(Context), 0 ) )( ConstantInt::get(Type::getInt32Ty(Context), offset ) );
  Value* pv = GetElementPtrInst::Create( ptr, vals.begin(), vals.end(), "storetoptr", bb );
  new StoreInst( v, pv, bb );
}

void LLVMGenerator::addParsedAssembly()
{
  SMDiagnostic error;
  M = ParseAssemblyString(mstr.c_str(), M, error, Context) ;
  if (!M)
  {
    error.Print("Module failed construction", outs());
    return;
  }
  mstr = "";
}

  delete jitdh;
  if (EE)
  {
    EE->freeMachineCodeForFunction(parse);
    EE->freeMachineCodeForFunction(getTrail);
    delete EE;
  }

