
#pragma once

#include <cassert>
#include <typeinfo>

#include <iostream>

using namespace std;

namespace PEG
{

template <class T>
class Handle
{
public:
  Handle()
  {
    refcount = 0; 
  }

  Handle( T* vp )
  { 
    refcount = 0; 
    init(vp); 
  }

  Handle (const Handle<T> &hp)
  {
    *this = hp;
  }

  template <class U>
  Handle (const Handle<U> &hp)
  {
    *this = hp;
  }

  ~Handle() 
  { 
    if (refcount)
    {
      release(); 
    }
  }

  void verify(const char* msg) const
  {
    if (!refcount)
    {
      throw msg;
    }
  }
  T& ref() const
  { 
    verify("ref() called on uninitialized handle");
    return **h;
  }
  T* get() const 
  { 
    verify("get() called on uninitialized handle");
    return *h; 
  }
  T** ptr() const 
  { 
    return h; 
  }
  T& operator*() const
  { 
    return ref(); 
  }
  T* operator->() const
  {
    return get();
  }
  unsigned int getRefcount() const 
  { 
    verify("getRefcount called on uninitialized handle");
    return *refcount; 
  }
  unsigned int* getRefcountPtr() const 
  { 
    return refcount; 
  }
  void assign( T& v ) 
  { 
    verify("assign called on uninitialized handle");
    **h = v;
  }
  void set( T* vp )
  {
    if (!refcount)
    {
      init(vp);
    }
    else 
    {
      *h = vp;
    }
  }
  void reset( T* vp )
  { 
    if (refcount) 
    { 
      release();
    }
    init(vp);
  }

  Handle<T>& operator=(Handle<T> const& h_in)
  {
    h = h_in.ptr();
    refcount = h_in.getRefcountPtr();
    if (refcount)
    {
      (*refcount)++;
    }
    return *this;
  }

  template <class U>
  Handle<T>& operator=(Handle<U> const& h_in)
  {
    h = reinterpret_cast<T**>(h_in.ptr());
    refcount = h_in.getRefcountPtr();
    if (refcount)
    {
      (*refcount)++;
    }
    return *this;
  }

  unsigned int* refcount;
  T** h;

private:

  void init( T* vp )
  {
    assert(!refcount);
    refcount = new unsigned int;
    *refcount = 1;
    h = new T*;
    *h = vp; 
  }
  void release()
  {
    assert(refcount && *refcount);
    if (*refcount == 1)
    {
      delete refcount;
      delete *h;
    } 
    else
    {
      (*refcount)--;
      refcount = 0;
    }
  }
};

template<class T, class U>
bool operator<(Handle<T> const& a, Handle<U> const& b)
{
  return a.ref() < b.ref();
}

}
