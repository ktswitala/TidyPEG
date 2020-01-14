
// Semantics.hpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#pragma once

namespace PEG
{
  
class Semantics
{
protected:
  virtual void Start();

  ParseResult presult;
};

}