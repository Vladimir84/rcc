// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v 1.8 2005/09/10 21:28:33 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.cc,v $
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <iostream>
#include <string>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include <support/DumpMacros.h>
#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>

#include "Annotation.h"

//*************************** Forward Declarations ***************************

//****************************************************************************


namespace RAnnot {


//****************************************************************************
// Environment
//****************************************************************************

Environment::Environment()
{
}


Environment::~Environment()
{
}


std::ostream&
Environment::dump(std::ostream& os) const
{
  beginObjDump(os, Environment);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    os << "(" << it->first << " --> " << it->second << ")\n";
  }
  endObjDump(os, Environment);
}


//****************************************************************************
// ExpressionInfo
//****************************************************************************

PropertyHndlT ExpressionInfo::ExpressionInfoProperty = "ExpressionInfo";

ExpressionInfo::ExpressionInfo()
{
}


ExpressionInfo::~ExpressionInfo()
{
}


std::ostream&
ExpressionInfo::dump(std::ostream& os) const
{
  beginObjDump(os, ExpressionInfo);
  SEXP definition = CAR(mDefn);
  dumpSEXP(os, definition);
  MySet_t::iterator var_iter;
  for(var_iter = mVars.begin(); var_iter != mVars.end(); ++var_iter) {
    (*var_iter)->dump(os);
  }
  endObjDump(os, ExpressionInfo);
}


//****************************************************************************
// TermInfo
//****************************************************************************

TermInfo::TermInfo()
{
}


TermInfo::~TermInfo()
{
}


std::ostream&
TermInfo::dump(std::ostream& os) const
{
  beginObjDump(os, TermInfo);
  endObjDump(os, TermInfo);
}


//****************************************************************************
// Var
//****************************************************************************

PropertyHndlT Var::VarProperty = "Var";

Var::Var()
{
}


Var::~Var()
{
}


std::ostream&
Var::dump(std::ostream& os) const
{
  beginObjDump(os, Var);
  dumpObj(os, mReachingDef);
  endObjDump(os, Var);
}

//****************************************************************************
// UseVar
//****************************************************************************

UseVar::UseVar()
{
  mUseDefType = Var_USE;
}

UseVar::~UseVar()
{
}

SEXP UseVar::getName() const
{
  return CAR(mSEXP);
}

std::ostream&
UseVar::dump(std::ostream& os) const
{
  beginObjDump(os,UseVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os,name);
  dumpVar(os,mUseDefType);
  dumpVar(os,mMayMustType);
  dumpVar(os,mScopeType);
  //dumpPtr(os,mReachingDef);
  dumpVar(os,mPositionType);
  endObjDump(os,UseVar);
}

//****************************************************************************
// DefVar
//****************************************************************************

DefVar::DefVar()
{
  mUseDefType = Var_DEF;
}

DefVar::~DefVar()
{
}

SEXP DefVar::getName() const
{
  if (mSourceType == DefVar_ASSIGN) {
    return CAR(mSEXP);
  } else if (mSourceType == DefVar_FORMAL) {
    return TAG(mSEXP);
  } else {
    assert(0);
    return R_NilValue;
  }
}

std::ostream&
DefVar::dump(std::ostream& os) const
{
  beginObjDump(os,DefVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os,name);
  dumpVar(os,mUseDefType);
  dumpVar(os,mMayMustType);
  dumpVar(os,mScopeType);
  // dumpPtr(os,mReachingDef);
  dumpVar(os,mSourceType);
  endObjDump(os,DefVar);
}

//****************************************************************************
// FuncVar
//****************************************************************************

FuncVar::FuncVar()
{
}


FuncVar::~FuncVar()
{
}


std::ostream&
FuncVar::dump(std::ostream& os) const
{
  beginObjDump(os, FuncVar);
  dumpVar(os, mIsReachingDefKnown);
  endObjDump(os, FuncVar);
}


//****************************************************************************
// Literal
//****************************************************************************

Literal::Literal()
{
}


Literal::~Literal()
{
}


std::ostream&
Literal::dump(std::ostream& os) const
{
  beginObjDump(os, Literal);
  endObjDump(os, Literal);
}


//****************************************************************************
// VarInfo
//****************************************************************************

VarInfo::VarInfo()
{
}


VarInfo::~VarInfo()
{
}


std::ostream&
VarInfo::dump(std::ostream& os) const
{
  beginObjDump(os, VarInfo);
  // FIXME: add implementation
  endObjDump(os, VarInfo);
}


//****************************************************************************
// FuncInfo
//****************************************************************************

PropertyHndlT FuncInfo::FuncInfoProperty = "FuncInfo";

FuncInfo::FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn) :
  mLexicalParent(lexParent),
  mFirstName(name),
  mDefn(defn),
  mRequiresContext(true),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(lexParent)
{
  mEnv = new Environment();
}


FuncInfo::~FuncInfo()
{
}


void FuncInfo::setRequiresContext(bool requiresContext) 
{ 
  mRequiresContext = requiresContext; 
}


bool FuncInfo::getRequiresContext() 
{ 
#if 1
  return mRequiresContext;
#else
  return false ; // for now 
#endif
}

SEXP FuncInfo::getArgs() 
{ 
  return CAR(fundef_args_c(mDefn)); 
}

void FuncInfo::insertMention(Var * v)
{
  mMentions.insert(v);
}

std::ostream&
FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpVar(os, mNumArgs);
  dumpVar(os, mHasVarArgs);
  dumpVar(os, mCName);
  dumpVar(os, mRequiresContext);
  dumpObj(os, mEnv);
  //dumpObj(os, mCFG);      // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, mFirstName);
  dumpSEXP(os, mDefn);
  dumpPtr(os, mLexicalParent);
  os << "Begin mentions:" << std::endl;
  for(mentions_iterator i = mMentions.begin(); i != mMentions.end(); ++i) {
    (*i)->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, FuncInfo);
}

//****************************************************************************
// Phi
//****************************************************************************

Phi::Phi()
{
}


Phi::~Phi()
{
}


std::ostream&
Phi::dump(std::ostream& os) const
{
  beginObjDump(os, Phi);
  // FIXME: add implementation
  endObjDump(os, Phi);
  
}


//****************************************************************************
// ArgInfo
//****************************************************************************

ArgInfo::ArgInfo()
{
}


ArgInfo::~ArgInfo()
{
}


std::ostream&
ArgInfo::dump(std::ostream& os) const
{
  beginObjDump(os, ArgInfo);
  endObjDump(os, ArgInfo);
}


//****************************************************************************
// FormalArg
//****************************************************************************

FormalArg::FormalArg()
{
}


FormalArg::~FormalArg()
{
}


std::ostream&
FormalArg::dump(std::ostream& os) const
{
  beginObjDump(os, FormalArg);
  // FIXME: add implementation
  endObjDump(os, FormalArg);
}


} // end of RAnnot namespace