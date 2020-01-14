
// ExpressionVisitor.cpp - TidyPEG
// This file is distributed under the terms of the Lesser GNU Public Licence
// Copyright 2010 Kenneth Switala 

#include "core/ExpressionVisitor.hpp"
#include "core/Expression.hpp"

namespace PEG
{

void Empty::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void SingleTerminal::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void RangeTerminal::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void AnyTerminal::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void NonTerminal::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Sequence::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Choice::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Repeat::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Not::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Plus::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

void Question::Accept( ExpressionVisitor& v ) const 
{
  v.Visit(*this);
} 

}
