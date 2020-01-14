
#pragma once

#include "useful/STDInclude.hpp"
#include "useful/LLVMInclude.hpp"

namespace PEG
{
  using namespace llvm;
  using namespace std;
  using namespace boost;

  template<class T>
  class ArrayIterator
  {
  public:
    char* start_element;
    char* current_element;
    char* end_element;
    uint element_size;

    ArrayIterator<T>& operator++(int) { current_element += element_size; return *this; }
    T* get() { return reinterpret_cast<T*>(current_element); } 
    bool end() { return (start_element==end_element); }
  };


  class JITDataHelper
  {
  public:
    JITDataHelper(ExecutionEngine*);


template <typename T>
shared_ptr<ArrayIterator<T> > GetIterator(void* base, const ArrayType* at) const
{
  shared_ptr<ArrayIterator<T> > ai( new ArrayIterator<T>() );

  ai->start_element = reinterpret_cast<char*>(base);
  ai->current_element = ai->start_element;
  const Type* ct = at->getElementType();
  ai->element_size = td->getTypeAllocSize( ct );
  ai->end_element = ai->start_element + ai->element_size * at->getNumElements();

  return ai;
}

template <typename T>
T* GetDataPtr(void* base, const Type* t, const uint idx) const
{
  vector<uint> tv;
  tv.push_back( idx );
  return GetDataPtr<T>(base, t, tv);
}

template <typename T>
T* GetDataPtr(void* base, const Type* t, const vector<uint>& v) const
{
  const Type* ct = t;
  uint dptr_offset = 0;
  for (uint i = 0, e = v.size(); i < e; i++)
  {
    if (ct->getTypeID() == Type::ArrayTyID)
    {
      const ArrayType* at = dynamic_cast<const ArrayType*>( ct ); 
      ct = at->getElementType();
      dptr_offset += v[i]*td->getTypeAllocSize( ct ) ;
    }
    else if (ct->getTypeID() == Type::StructTyID)
    {
      const StructType* st = dynamic_cast<const StructType*>( ct );
      const StructLayout* sl = td->getStructLayout( st );
      ct = st->getElementType( v[i] );
      dptr_offset += sl->getElementOffset( v[i] );
    }
    else
    {
      throw "No further indexing allowed in JITDataHelper::GetDataPtr";
    }
  }
  char* c = reinterpret_cast<char*>(base)+dptr_offset;
  return reinterpret_cast<T*>(c);
}

    const TargetData* td;
  };
}
