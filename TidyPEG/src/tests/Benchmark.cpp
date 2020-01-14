
// Test.cpp
// Copyright 2010 Kenneth Switala 

#include "useful/STDInclude.hpp"
#include "useful/LLVMInclude.hpp"
#include "core/Grammar.hpp"
#include "parse/Generator.hpp"
#include "semantics/ParseResult.hpp"
#include "semantics/BootstrapSemantics.hpp"

using namespace std;
using namespace llvm;
using namespace PEG;

class TestPEG
{
public:

Grammar& bootstrap;
LLVMGenerator llvmgen;
MemoryBuffer* buf;
const char* f;

TestPEG(const char* f_in, Grammar& g) : bootstrap(g), llvmgen( bootstrap )
{
  std::cout << "Reading: " << f << endl;
  f = f_in;
  buf = MemoryBuffer::getFile(f);
  std::cout << "Read: " << f << endl;
}

void BenchMarkFile()
{
  Timer many("manyrun");
  many.startTimer();

  ParseResult res;
  std::cout << "Speed test for " << f << ": " << std::endl;
  for (uint i = 1; i <= 1000; i++)
  {
    if (!(i % 100))
    {
      std::cout << i/10 << " ";
      std::cout.flush();
    }
    llvmgen.runParse( *buf, res );
  }
  many.stopTimer();
  std::cout << std::endl;

  std::cout << "Average run for " << f << ": " << many.getProcessTime() << " ms. (";
  std::cout << (buf->getBufferSize() / 1024) / (many.getProcessTime()/1000) << " KiB/sec)." << std::endl;
  std::cout << "Parse tree memory usage: " << res.MemoryUsed() / (1024.0*1024) << " MiB. (" 
            << res.MemoryUsed() / buf->getBufferSize() << " bytes memory/bytes input)" << std::endl;
}

void TestFile()
{
  MemoryBuffer* buf = MemoryBuffer::getFile(f);
  Timer first("firstrun");
  first.startTimer();

  ParseResult res;
  bool parseres = llvmgen.runParse( *buf, res );
  first.stopTimer();

  if (!parseres)
  {
    std::cout << "Parse failed" << endl;
    return;
  }

  //PEGSemantics sem(res);
  //try
  //{
  //  sem.Start();
  //} 
  //catch(string s)
  // {
  //  std::cout << s << std::endl;
  //  throw;
  // }
}

};

int main()
{
  std::cout << "Initialize" << endl;
  InitializeNativeTarget();

  std::cout << "Bootstrap" << endl;
  Grammar& bootstrap = Grammar::InitializePegpeg();

try
{
  std::cout << "Readfile" << endl;
  TestPEG tpeg( "./grammars/peg.peg", bootstrap );

  tpeg.TestFile();
  tpeg.BenchMarkFile();
}
catch(const char* cc)
{
  std::cout << cc << endl;
}

  llvm_shutdown();

  return 0;
}

