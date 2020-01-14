
#include "useful/JITDataHelper.hpp"

using namespace std;
using namespace boost;

namespace PEG
{

JITDataHelper::JITDataHelper(ExecutionEngine* ee)
{
  td = ee->getTargetData();
}

}
