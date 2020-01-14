
// Common.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

#include "useful/STDInclude.hpp"

namespace PEG
{

template<class T>
class MemoryBlock
{
private:
  T* pool;
  size_t sz;
  unsigned int position;
public:
  MemoryBlock()
  {
    sz = 0;
    pool = 0;
  }
  MemoryBlock(size_t size)
  {
    sz = 0;
    pool = 0;
    reset( size );
  }
  ~MemoryBlock()
  {
    free();
  }
  T* malloc(size_t size)
  {
    if (position+size >= sz)
    {
      resize( sz * 2 );
    }
    T* ret = new(&pool[position]) T();
    for (unsigned int i = 0; i < size; i++)
    {
      new(&pool[position]) T();
      position++;
    }
    return ret;
  }
  void free()
  {
    if (pool) 
    {
      for (unsigned int i = 0; i < position; i++)
      {
        T* delme = &pool[i];
        delme->~T();
      }
      ::free(pool);
    }
  }
  unsigned int used()
  {
    return (position*sizeof(T));
  }
  void reset()
  {
    position = 0;
  }
  void resize(size_t size) 
  {
    if (size > sz)
    {
      T* newpool = reinterpret_cast<T*>(::malloc( size * sizeof(T) ) );
      std::copy( pool, pool+used(), newpool );
      ::free(pool);
      pool = newpool;
      sz = size;
    }
  }
};

}
