/*! \file
  
  \brief DataflowSet that maps a differentiable use loc to the def locs 
         that depend on it vice versa.

  \authors Michelle Strout 
  \version $Id: DepDFSet.hpp,v 1.2 2005/06/10 02:32:01 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DepDDFSet_h
#define DepDDFSet_h

#include <iostream>
#include <map>

// abstract interface that this class implements
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/Utils/Iterator.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>

namespace OA {
  namespace Activity {

class DepIterator;

/*!
   A set of differentiable dependence pairs, <useLoc,defLoc>.
   Dependences to self are implicit unless removed with removeDep.
 */
    class DepDFSet : public virtual DataFlow::DataFlowSet {
        
public:
  DepDFSet();
  //! copy constructor
  DepDFSet(const DepDFSet &other); 
  ~DepDFSet() {}

  //! assignment
  DepDFSet& operator=(const DepDFSet &other);

  //*****************************************************************
  // DataFlowSet Interface Implementation
  //*****************************************************************
  OA_ptr<DataFlow::DataFlowSet> clone();
  
  // param for these can't be const because will have to 
  // dynamic cast to specific subclass
  bool operator ==(DataFlow::DataFlowSet &other) const;

  bool operator !=(DataFlow::DataFlowSet &other) const;

  //*****************************************************************
  // Methods specific to DepDFSet
  //*****************************************************************

  
  //! unions this DepDFSet with other and stores result in this
  DepDFSet& setUnion(DataFlow::DataFlowSet &other);

  //! If <a,b> in *this and <b,c> in other then add <a,c> to *this
  DepDFSet& compose(DataFlow::DataFlowSet &other);
  //OA_ptr<DepDFSet> compose(OA_ptr<DepDFSet> other);

  //! get an iterator over deps
  OA_ptr<DepIterator> getDepIterator() const;

  //! get an iterator over AliasTags that depend on the given 
  //! differentiable use
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > getDefsIterator(int use) ;

  //! get an iterator over differentiable  AliasTags that the
  //! given def depends on
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > getUsesIterator(int def);

  //! intersects this DepDFSet with other into self
//  DepDFSet& setIntersect(DepDFSet &other);

//  DepDFSet& setDifference(DepDFSet &other);

  //*****************************************************************
  // Annotation Interface
  //*****************************************************************
  void output(OA::IRHandlesIRInterface& ir);

  //*****************************************************************
  // Output
  //*****************************************************************

  void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);
  void dump(std::ostream &os);
  void dump(std::ostream &os) const;

  //*****************************************************************
  // Construction
  //*****************************************************************
  
  void insertDep(int use, int def);

  //! needed to indicate when implicit reflexive pairs (eg. <a,a>)
  //! are not in the Dep set due to a must define of a
  void removeImplicitDep(int use, int def);

  typedef std::map<int, DataFlow::DataFlowSetImpl<int> >
      AliasTagToAliasTagDFSetMap;
  
  //! helper function that determines if the given location overlaps
  //! with anything in the set of AliasTags whose implicit pair AliasTag,AliasTag
  //! has been killed due to a must def
  bool isImplicitRemoved(int aTag);

  //! I dont know what should be consider as size of DepDFSet
  //! But here is dummy function for pure abstract function in DataFlowSet
  int size() { return 0; }

private:
  //! helper function for iterators, makes all implicit deps explicit
  //! except those that have been removed explicitly
  //void makeImplicitExplicit();
  //bool mMakeImplicitExplicitMemoized;

  friend class DepIterator;

  // It is implicitly assumed that dep <a,a> is in each DepDFSet
  // for all locations.  This is a set of locations where this
  // implicit dep has been removed with removeImplicitDep.
  // If <a,a> is then explicitly inserted with insertDep, then
  // anything that overlaps with a will be removed from this set
  DataFlow::DataFlowSetImpl<int> mImplicitRemoves;

  AliasTagToAliasTagDFSetMap mUseToDefsMap;
  AliasTagToAliasTagDFSetMap mDefToUsesMap;

  // maintain sets of the uses and defs as well
  DataFlow::DataFlowSetImpl<int> mUses;
  DataFlow::DataFlowSetImpl<int> mDefs;

};


class DepIterator : public Iterator {
   public :
      DepIterator(OA_ptr<DepDFSet> dfSet) : mDFSet(dfSet) {

         mDFSet = dfSet;
         mIter = mDFSet->mUseToDefsMap.begin();
      } 

      int use() const
      { return mIter->first; }

      DataFlow::DataFlowSetImpl<int> def() const
      { return mIter->second; }

      virtual bool isValid() const {
         return mIter !=  mDFSet->mUseToDefsMap.end();
      }

      virtual void operator++() {
         mIter++;
      }

      virtual void reset() {
         mIter = mDFSet->mUseToDefsMap.begin();
      }

   private:
      OA_ptr<DepDFSet> mDFSet;
      DepDFSet::AliasTagToAliasTagDFSetMap::iterator mIter;

};



  } // end of DataFlow namespace
} // end of OA namespace

#endif
