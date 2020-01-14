
#include "useful/LLVMInclude.hpp"
#include "useful/JITDataHelper.hpp"

namespace PEG
{
  using namespace llvm;
  
  struct JumpHelper
  {
    JumpHelper() { jumps.push_back(0); }

    std::vector<BasicBlock*> jumps;

    void generate(SwitchInst*);
    uint Add(BasicBlock*);
  };

  struct StackHelper
  {
    Value* stackP;
    Value* stackPosP;
    std::string name;

    StackHelper(std::string n);
    void Init(const Type*);
    void Init(Value*);
    Value* CalcElement();
    Value* CalcElement(Value* stackpos);
    Value* CalcElementById(Value* idpos);
    Value* PrepareStack();
    void ReleaseStack();
  };

  class BlockHelper
  {
    void SetBB(BasicBlock*);
    BasicBlock* bb;
    LLVMContext& Context;
  };
  
  class ModuleHelper
  {
  };

  void addParsedAssembly();
  void StoreToPtr(Value*, uint, Value*);
  void setSwitches(SwitchInst*, std::vector<BasicBlock*>&);

  Module* M;
  ExecutionEngine* EE;
  ExistingModuleProvider* OMP;
  JITDataHelper* jitdh;

}