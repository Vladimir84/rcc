/* $Id: StackableIterator.h,v 1.1 2005/08/31 05:15:38 johnmc Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// StackableIterator.h
//
//   a base set of functionality for iterators that can be used with the
//   IteratorStack abstraction to traverse nested structures 
//
// Author: John Mellor-Crummey                                
//
// Creation Date: October 1993
//
// Modification History:
//  November 1994 -- John Mellor-Crummey
//    -- postincrement implemented by calling the preincrement operator
//    -- add Dump/DumpUpCall methods to support debugging
//    -- add ClassName method to support debugging
//    -- const where appropriate 
//    
//***************************************************************************


#ifndef StackableIterator_h
#define StackableIterator_h

#if 0
#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#endif

#ifndef ClassName_h
#include <include/ClassName.h>
#endif



//***************************************************************************
// class StackableIterator
//***************************************************************************

class StackableIterator {
public:
  StackableIterator();
  virtual ~StackableIterator();

  //----------------------------------------------------------------------
  // upcall to get the value of the current element in the iteration from 
  // a derived iterator 
  //----------------------------------------------------------------------
  virtual void *CurrentUpCall() const = 0;

  //----------------------------------------------------------------------
  // upcall to advance the iteration
  //----------------------------------------------------------------------
  virtual void operator++() = 0; // prefix increment
  void operator++(int);          // postincrement via preincrement operator

  //----------------------------------------------------------------------
  // predicate to test if the value returned by CurrentUpCall is valid.
  // supplied default implementation returns true if the value is non-zero 
  //----------------------------------------------------------------------
  virtual bool IsValid() const; 

  //----------------------------------------------------------------------
  // upcall to restart the iteration at the beginning
  //----------------------------------------------------------------------
  virtual void Reset() = 0;

#if 0
  //----------------------------------------------------------------------
  // dump essential state (class name, this pointer, currrent iterate) 
  // and invoke DumpUpCall to report interesting state of derived class
  //----------------------------------------------------------------------
  void Dump();
#endif

  CLASS_NAME_FDEF(StackableIterator); // virtual ClassName()

#if 0
private:
  //----------------------------------------------------------------------
  // upcall to dump any interesting state of a derived class 
  //----------------------------------------------------------------------
  virtual void DumpUpCall();
#endif
};

CLASS_NAME_EDEF(StackableIterator);

#endif

