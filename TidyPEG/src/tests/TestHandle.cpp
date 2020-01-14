
#include <string>
#include <iostream>
#include "Handle.hpp"

using namespace std;
using namespace PEG;

// A class that does nothing but tell you what it is doing.
class NoisyClass
{
public:
  NoisyClass(string name_in)
  {
    name = name_in;
    *complaints << "NoisyClass() " << name << endl;
  }
  void report() const
  {
    *complaints << name << " reporting in!" << endl;
  }
  ~NoisyClass()
  {
    *complaints << "~NoisyClass() " << name << endl;
  }
  static void SetComplaints(ostream* c)
  {
    complaints = c;
  }
  string name;
  static ostream* complaints;
};

ostream* NoisyClass::complaints = 0;

int main()
{
  NoisyClass::SetComplaints(&cout);

  Handle<NoisyClass> h1;
  Handle<NoisyClass> h2(new NoisyClass("nc1"));
  Handle<const NoisyClass> h3(new NoisyClass("nc2"));

  assert(h2.getRefcount() == 1);
  {
    Handle<NoisyClass> h4(h2);
    assert(h2.getRefcount() == 2);

    Handle<const NoisyClass> h5(h2);
    assert(h2.getRefcount() == 3);
  }
  assert(h2.getRefcount() == 1);

  h2->report();
  (*h2).report();
  h3->report();
  (*h3).report();
}

