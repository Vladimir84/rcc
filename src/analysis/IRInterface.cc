// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: IRInterface.cc
//
// Derived class of OpenAnalysis base classes; defines the interface
// between R and OA.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/ExprTreeBuilder.h>
#include <analysis/HandleInterface.h>
#include <analysis/SimpleIterators.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarRefFactory.h>
#include <analysis/Utils.h>

#include <support/RccError.h>

#include <analysis/IRInterface.h>

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

//--------------------------------------------------------
// Procedures and call sites
//--------------------------------------------------------

/// Given a ProcHandle, return an IRRegionStmtIterator for the
/// procedure.
OA_ptr<IRRegionStmtIterator> R_IRInterface::procBody(ProcHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(fundef_body_c(make_sexp(h))));
  return ptr;
}

//--------------------------------------------------------
// Statements: General
//--------------------------------------------------------

/// Return statements are allowed in R.
bool R_IRInterface::returnStatementsAllowed() { return true; }

/// Local R-specific function: return the CFG type (loop, conditional,
/// etc.) of an SEXP.
CFG::IRStmtType getSexpCfgType(SEXP e) {
  switch(TYPEOF(e)) {
  case NILSXP:  // expressions as statements
  case REALSXP:
  case LGLSXP:
  case STRSXP:
  case SYMSXP:
    return CFG::SIMPLE;
    break;
  case LANGSXP:
    if (is_fundef(e) || is_paren_exp(e) || is_curly_list(e)) {
      return CFG::COMPOUND;
    } else if (is_loop(e)) {
      return CFG::LOOP;
    } else if (is_if(e)) {
      return CFG::STRUCT_TWOWAY_CONDITIONAL;
    } else if (is_return(e)) {
      return CFG::RETURN;
    } else if (is_break(e) || is_stop(e)) {
      return CFG::BREAK;
    } else if (is_next(e)) {
      return CFG::LOOP_CONTINUE;
    } else {
      return CFG::SIMPLE;
    }
    break;
  default:
    assert(0);
  }
  return CFG::SIMPLE; // never reached
}

/// Given a statement handle, return its IRStmtType.
CFG::IRStmtType R_IRInterface::getCFGStmtType(StmtHandle h) {
  return getSexpCfgType(CAR(make_sexp(h)));
}

/// Given a statement, return a label (or StmtHandle(0) if there is no
/// label associated with the statement). No statements have labels in
/// R.
StmtLabel R_IRInterface::getLabel(StmtHandle h) {
  return 0;
}

/// Given a compound statement (which contains a list of statements),
/// return an IRRegionStmtIterator for the statements.
OA_ptr<IRRegionStmtIterator> R_IRInterface::getFirstInCompound(StmtHandle h) {
  SEXP cell = make_sexp(h);
  SEXP e = CAR(cell);
  OA_ptr<IRRegionStmtIterator> ptr;
  if (is_curly_list(e)) {
    // use the iterator that doesn't take a cell
    ptr = new R_RegionStmtListIterator(curly_body(e));
  } else if (is_paren_exp(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(paren_body_c(e)));
  } else if (is_fundef(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(fundef_body_c(e)));
#if 0
  } else if (is_loop(e)) {
    ptr = new R_RegionStmtIterator(make_stmt_h(loop_body_c(e)));
#endif
  } else {
    rcc_error("getFirstInCompound: unrecognized statement type");
  }
  return ptr;
}
  
//--------------------------------------------------------
// Loops
//--------------------------------------------------------

/// Given a loop statement, return an IRRegionStmtIterator for the loop body.
OA_ptr<IRRegionStmtIterator> R_IRInterface::loopBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(loop_body_c(CAR(make_sexp(h)))));
  return ptr;
}

/// Given a loop statement, return the loop header statement.  This 
/// would be the initialization statement in a C 'for' loop, for example.
///
/// This doesn't exactly exist in R. Currently just returning the whole
/// compound statement pointer so later analyses can parse it.
StmtHandle R_IRInterface::loopHeader(StmtHandle h) {
  // TODO: signal that this is a header for use/def analysis
  return h;
}

/// Given a loop statement, return the increment statement.
///
/// This doesn't exactly exist in R. Currently just returning the whole
/// compound statement pointer so later analyses can parse it.
StmtHandle R_IRInterface::getLoopIncrement(StmtHandle h) {
  // TODO: signal that this is the increment for use/def analysis
  return h;
}

/// Given a loop statement, return:
/// 
/// True: If the number of loop iterations is defined
/// at loop entry (i.e. Fortran semantics).  This causes the CFG builder 
/// to add the loop statement representative to the header node so that
/// definitions from inside the loop don't reach the condition and increment
/// specifications in the loop statement.
///
/// False: If the number of iterations is not defined at
/// entry (i.e. C semantics), we add the loop statement to a node that
/// is inside the loop in the CFG so definitions inside the loop will 
/// reach uses in the conditional test. For C style semantics, the 
/// increment itself may be a separate statement. if so, it will appear
/// explicitly at the bottom of the loop. 
bool R_IRInterface::loopIterationsDefinedAtEntry(StmtHandle h) {
  return true;
}

/// Given a structured two-way conditional statement, return an
/// IRRegionStmtIterator for the "true" part (i.e., the statements
/// under the "if" clause).
OA_ptr<IRRegionStmtIterator> R_IRInterface::trueBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(if_truebody_c(CAR(make_sexp(h)))));
  return ptr;
}

/// Given a structured two-way conditional statement, return an
/// IRRegionStmtIterator for the "else" part (i.e., the statements
/// under the "else" clause).
OA_ptr<IRRegionStmtIterator> R_IRInterface::elseBody(StmtHandle h) {
  OA_ptr<IRRegionStmtIterator> ptr;
  ptr = new R_RegionStmtIterator(make_stmt_h(if_falsebody_c(CAR(make_sexp(h)))));
  return ptr;
}

//--------------------------------------------------------
// Structured multiway conditionals
//--------------------------------------------------------

// Given a structured multi-way branch, return the number of cases.
// The count does not include the default/catchall case.
int R_IRInterface::numMultiCases(StmtHandle h) {
  rcc_error("numMultiCases: multicase branches don't exist in R");
  return -1;
}

// Given a structured multi-way branch, return an
// IRRegionStmtIterator* for the body corresponding to target
// 'bodyIndex'. The n targets are indexed [0..n-1].  The user must
// free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::multiBody(StmtHandle h, int bodyIndex) {
  rcc_error("multiBody: multicase branches don't exist in R");
  OA_ptr<IRRegionStmtIterator> dummy;
  return dummy;
}

// Given a structured multi-way branch, return true if the cases have
// implied break semantics.  For example, this method would return false
// for C since one case will fall-through to the next if there is no
// explicit break statement.  Matlab, on the other hand, implicitly exits
// the switch statement once a particular case has executed, so this
// method would return true.
bool R_IRInterface::isBreakImplied(StmtHandle multicond) {
  rcc_error("isBreakImplied: multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return true if the body 
// corresponding to target 'bodyIndex' is the default/catchall/ case.
bool R_IRInterface::isCatchAll(StmtHandle h, int bodyIndex) {
  rcc_error("isCatchAll: multicase branches don't exist in R");
  return false;
}

// Given a structured multi-way branch, return an IRRegionStmtIterator*
// for the body corresponding to default/catchall case.  The user
// must free the iterator's memory via delete.
OA_ptr<IRRegionStmtIterator> R_IRInterface::getMultiCatchall (StmtHandle h) {
  rcc_error("getMultiCatchall: multicase branches don't exist R");
  OA_ptr<IRRegionStmtIterator> dummy;
  return dummy;
}

/// Given a structured multi-way branch, return the condition
/// expression corresponding to target 'bodyIndex'. The n targets are
/// indexed [0..n-1].
ExprHandle R_IRInterface::getSMultiCondition(StmtHandle h, int bodyIndex) {
  rcc_error("getSMultiCondition: multicase branches don't exist R");
  return 0;
}


//--------------------------------------------------------
// Unstructured two-way conditionals: 
//--------------------------------------------------------

// Given an unstructured two-way branch, return the label of the
// target statement.  The second parameter is currently unused.
StmtLabel R_IRInterface::getTargetLabel(StmtHandle h, int n) {
  rcc_error("getTargetLabel: unstructured two-way branches don't exist in R");
  return 0;
}

//--------------------------------------------------------
// Unstructured multi-way conditionals
// TODO: Review all of the multi-way stuff.
//--------------------------------------------------------

// Given an unstructured multi-way branch, return the number of targets.
// The count does not include the optional default/catchall case.
int R_IRInterface::numUMultiTargets(StmtHandle h) {
  rcc_error("numUMultiTargets: unstructured multi-way branches don't exist in R");
  return -1;
}

// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
StmtLabel R_IRInterface::getUMultiTargetLabel(StmtHandle h, int targetIndex) {
  rcc_error("getUMultiTargetLabel: unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return label of the target
// corresponding to the optional default/catchall case.  Return 0
// if there is no default target.
StmtLabel R_IRInterface::getUMultiCatchallLabel(StmtHandle h) {
  rcc_error("getUMultiCatchallLabel: unstructured multi-way branches don't exist in R");
  return 0;
}

// Given an unstructured multi-way branch, return the condition
// expression corresponding to target 'targetIndex'. The n targets
// are indexed [0..n-1].
// multiway target condition 
ExprHandle R_IRInterface::getUMultiCondition(StmtHandle h, int targetIndex) {
  rcc_error("getUMultiCondition: unstructured multi-way branches don't exist in R");
  return 0;
}

//----------------------------------------------------------------------
// Information for building call graphs
//----------------------------------------------------------------------

/// Given a subprogram return an IRStmtIterator for the entire
/// subprogram
OA_ptr<IRStmtIterator> R_IRInterface::getStmtIterator(ProcHandle h) {
  return procBody(h);
}

/// Return an iterator over all of the callsites in a given stmt
OA_ptr<IRCallsiteIterator> R_IRInterface::getCallsites(StmtHandle h) {
  OA_ptr<IRCallsiteIterator> iter; iter = new R_IRCallsiteIterator(h);
  return iter;
}

/// Given a procedure call create a memory reference expression
/// to describe that call.  For example, a normal call is
/// a NamedRef.  A call involving a function ptr is a Deref.
OA_ptr<MemRefExpr> R_IRInterface::getCallMemRefExpr(CallHandle h) {
  SEXP e = make_sexp(h);
  OA_ptr<MemRefExpr> mre;
  if (is_var(call_lhs(e))) {
    SymHandle sym = make_sym_h(call_lhs(e));
    mre = new NamedRef(MemRefExpr::USE, sym);
    //    mre = new Deref(MemRefExpr::USE, mre);
  } else {
    rcc_warn("getCallMemRefExpr: Call graph interface for calls with non-symbol LHS not yet implemented");
    throw AnalysisException();
  }
  return mre;
}

//----------------------------------------------------------------------
// Information for solving call graph data flow problems
//----------------------------------------------------------------------

OA_ptr<IRCallsiteParamIterator> R_IRInterface::getCallsiteParams(CallHandle h) {
  SEXP args = call_args(make_sexp(h));
  
  // create wrapper around R_ListIterator that iterates through the
  // params, returning ExprHandles instead of SEXPs
  class R_ParamIterator : public IRCallsiteParamIterator {
  public:
    R_ParamIterator(SEXP _exp) : m_list_iter(_exp) {
    }

    ExprHandle current() const {
      return make_expr_h(m_list_iter.current());
    }

    bool isValid() const {
      return m_list_iter.isValid();
    }

    void operator++() {
      ++m_list_iter;
    }

    void reset() {
      m_list_iter.reset();
    }
    
  private:
    R_ListIterator m_list_iter;
  };

  OA_ptr<IRCallsiteParamIterator> iter; iter = new R_ParamIterator(args);
  return iter;
}

SymHandle R_IRInterface::getFormalForActual(ProcHandle caller, CallHandle call,
					    ProcHandle callee, ExprHandle param)
{
  // TODO: handle named arguments

  // param is the nth actual arg of call: find n
  SEXP actual = make_sexp(param);
  SEXP args = call_args(make_sexp(call));
  int n = 1;
  R_ListIterator li(args);
  for( ; li.isValid(); ++li) {
    if (li.current() == actual) break;
    n++;
  }
  if (!li.isValid()) {
    rcc_error("getFormalForActual: param not found");
  }
    
  // get nth formal of callee
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(callee));
  return make_sym_h(TAG(fi->get_arg(n)));
}

OA_ptr<OA::Location> R_IRInterface::getLocation(ProcHandle p, SymHandle s) {
  SymbolTable * table = getProperty(SymbolTable, make_sexp(p));
  VarInfo * vi = (*table)[make_sexp(s)];

  OA_ptr<OA::Location> loc;
  // NamedLoc constructor's second argument: true = local, false = global
  //
  // we conservatively say global here because we don't know if it
  // might be referenced in a child scope
  loc = new NamedLoc(s, false);
  // TODO: need to give information about possible overlap
  return loc;
}

OA_ptr<ExprTree> R_IRInterface::getExprTree(ExprHandle h) {
  return ExprTreeBuilder::get_instance()->build(make_sexp(h));
}
  
/// from CalleeToCallerVisitorIRInterface
/// Given a MemRefHandle return an iterator over
/// MemRefExprs that describe this memory reference
///
/// Michelle says there's only one MemRefExpr per MemRefHandle in
/// almost all cases. Here, just return a singleton iterator
/// containing the MemRefExpr.
OA_ptr<MemRefExprIterator> R_IRInterface::getMemRefExprIterator(MemRefHandle h) {
  OA_ptr<MemRefExprIterator> iter;
  SEXP cell = make_sexp(h);
  if (is_var(CAR(cell))) {
    OA_ptr<MemRefExpr> mre; mre = new NamedRef(MemRefExpr::USE, make_sym_h(CAR(cell)));
    //    mre = new AddressOf(MemRefExpr::USE, mre);
    iter = new R_SingletonMemRefExprIterator(mre);
  } else {
    // TODO
    rcc_warn("getMemRefExprIterator: call graph interface not yet implemented for non-symbol callees");
    throw AnalysisException();
  }
  return iter;
}

/// from SideEffectIRInterface
/// Return a list of all the target memory reference handles that appear
/// in the given statement.
OA_ptr<MemRefHandleIterator> R_IRInterface::getDefMemRefs(StmtHandle h) {
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  // For each variable, insert only if it's a def
  OA_ptr<R_VarRefSet> defs; defs = new R_VarRefSet;
  VarRefFactory * fact = VarRefFactory::get_instance();
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = stmt_info->begin_vars(); var_iter != stmt_info->end_vars(); ++var_iter) {
    if ((*var_iter)->getUseDefType() == Var::Var_DEF) {
      OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref((*var_iter)->getMention_c());
      defs->insert_ref(bvr);
    }
  }
  OA_ptr<MemRefHandleIterator> retval;
  retval = new R_UseDefAsMemRefIterator(defs->get_iterator());
  return retval;
}

/// from SideEffectIRInterface
/// Return a list of all the source memory reference handles that appear
/// in the given statement.
OA_ptr<MemRefHandleIterator> R_IRInterface::getUseMemRefs(StmtHandle h) {
  ExpressionInfo * stmt_info = getProperty(ExpressionInfo, make_sexp(h));
  assert(stmt_info != 0);

  // For each variable, insert only if it's a use
  OA_ptr<R_VarRefSet> uses; uses = new R_VarRefSet;
  VarRefFactory * fact = VarRefFactory::get_instance();
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = stmt_info->begin_vars(); var_iter != stmt_info->end_vars(); ++var_iter) {
    if ((*var_iter)->getUseDefType() == Var::Var_USE) {
      OA_ptr<R_BodyVarRef> bvr; bvr = fact->make_body_var_ref((*var_iter)->getMention_c());
      uses->insert_ref(bvr);
    }
  }
  OA_ptr<MemRefHandleIterator> retval;
  retval = new R_UseDefAsMemRefIterator(uses->get_iterator());
  return retval;
}

/// from InterSideEffectIRInterface
/// For the given callee subprocedure symbol return side-effect results
/// Can only indicate that the procedure has no side effects, has
/// side effects on unknown locations, or on global locations.
/// Can't indicate subprocedure has sideeffects on parameters because
/// don't have a way to get mapping of formal parameters to actuals
/// in caller.
// Right now, using the default conservative approximation from InterSideEffectIRInterfaceDefault
//OA_ptr<SideEffect::SideEffectStandard> R_IRInterface::getSideEffect(ProcHandle, SymHandle) {
//  // TODO
//  rcc_warn("getSideEffect: call graph interface not yet implemented");
//  throw AnalysisException();
//}

//------------------------------------------------------------
// Alias information
//------------------------------------------------------------

/// Return an iterator over all the memory reference handles that appear
/// in the given statement.  Order that memory references are iterated
/// over can be arbitrary.
OA_ptr<MemRefHandleIterator> R_IRInterface::getAllMemRefs(StmtHandle stmt) {
  ExpressionInfo * ei = getProperty(ExpressionInfo, make_sexp(stmt));
  OA_ptr<MemRefHandleIterator> iter; iter = new R_CallMemRefHandleIterator(ei);
  return iter;
}

/// Given a statement, return its Alias::IRStmtType
Alias::IRStmtType R_IRInterface::getAliasStmtType(StmtHandle h) {
  return Alias::ANY_STMT;  // not a pointer assignment (R doesn't have pointers)
}

/// If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
/// pairs where there is a source and target such that target
OA_ptr<Alias::PtrAssignPairStmtIterator> R_IRInterface::getPtrAssignStmtPairIterator(StmtHandle stmt) {
  // we have no PTR_ASSIGN_STMTs, so return null
  OA_ptr<Alias::PtrAssignPairStmtIterator> empty;
  return empty;
}

/// Return an iterator over <int, MemRefExpr> pairs
/// where the integer represents which formal parameter 
/// and the MemRefExpr describes the corresponding actual argument. 
OA_ptr<Alias::ParamBindPtrAssignIterator> R_IRInterface::getParamBindPtrAssignIterator(CallHandle call) {
  // TODO
  rcc_warn("getParamBindPtrAssignIterator: Alias interface not yet implemented");
  throw AnalysisException();
  OA_ptr<Alias::ParamBindPtrAssignIterator> dummy;
  return dummy;
}

/// Return the symbol handle for the nth formal parameter to proc
/// Number starts at 0 and implicit parameters should be given
/// a number in the order as well.  This number should correspond
/// to the number provided in getParamBindPtrAssign pairs
/// Should return SymHandle(0) if there is no formal parameter for 
/// given num
SymHandle R_IRInterface::getFormalSym(ProcHandle h, int n) {
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(h));
  if (n >= fi->get_num_args()) {
    return SymHandle(0);
  } else {
    SEXP sym = TAG(fi->get_arg(n + 1));  // FuncInfo gives 1-based params
    return make_sym_h(sym);
  }
}

/// Given the callee symbol returns the callee proc handle
ProcHandle R_IRInterface::getProcHandle(SymHandle sym) {
  // TODO
  rcc_warn("getProcHandle: Alias interface not yet implemented");
  throw AnalysisException();
  return ProcHandle(0);
}

/// Given a procedure return associated SymHandle
SymHandle R_IRInterface::getSymHandle(ProcHandle h) {
  // TODO
  rcc_warn("getSymHandle: Alias interface not yet implemented");
  throw AnalysisException();
  return SymHandle(0);
}

//--------------------------------------------------------
// Obtain uses and defs for SSA
//--------------------------------------------------------

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getDefs(StmtHandle h) {
  // TODO
  rcc_warn("getDefs: SSA interface not yet implemented");
  throw AnalysisException();
  OA_ptr<SSA::IRUseDefIterator> dummy;
  return dummy;
}

OA_ptr<SSA::IRUseDefIterator> R_IRInterface::getUses(StmtHandle h) {
  // TODO
  rcc_warn("getUses: SSA interface not yet implemented");
  throw AnalysisException();
  OA_ptr<SSA::IRUseDefIterator> dummy;
  return dummy;
}

/// from IRHandlesIRInterface
void R_IRInterface::dump(StmtHandle h, ostream &os) {
  Rf_PrintValue(CAR(make_sexp(h)));
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(h));
  if (annot) {
    annot->dump(os);
  }
}

/// from IRHandlesIRInterface
void R_IRInterface::dump(MemRefHandle h, ostream &stream) {
  Rf_PrintValue(make_sexp(h));
}

//--------------------------------------------------------
// Symbol Handles
//--------------------------------------------------------

/// To build call graphs, we may need different procedures to return
/// different names here.
///
/// TODO: R procedures are first class. We need different procedures
/// to have different "names" even if they happen to be bound to the
/// same symbol. For now we're just giving the first name assigned.
SymHandle R_IRInterface::getProcSymHandle(ProcHandle h) {
  // TODO: remove the zero check; shouldn't be zero
  if (h == ProcHandle(0)) {
    return make_sym_h(Rf_install("<null procedure: bug in OA>"));
  }
  // TODO: make it easier to do this
  RProp::PropertySet::const_iterator iter = analysisResults.find(FuncInfo::handle());
  if (iter != analysisResults.end() && iter->second->is_computed()) {
    // if FuncInfo already exists, then use it
    // (otherwise, FuncInfoAnnotationMap would go into an infinite loop)
    FuncInfo * fi = getProperty(FuncInfo, make_sexp(h));
    return make_sym_h(fi->get_first_name());
  } else {
    return make_sym_h(Rf_install("<procedure>"));
  }
}

// TODO: symbols in different scopes should be called
// different, even if they have the same name
SymHandle R_IRInterface::getSymHandle(LeafHandle h) {
  return make_sym_h(make_sexp(h));
}

//------------------------------------------------------------
// Param bindings (ParamBindingsIRInterface)
//------------------------------------------------------------

// TODO: not enough information!
bool R_IRInterface::isParam(OA::SymHandle h) {
  return false;
}


// TODO: fill these in
//
// We need some way to convince the R system to give us a string for
// output instead of just spitting it out.
std::string R_IRInterface::toString(ProcHandle h) {
  return "";
}

std::string R_IRInterface::toString(StmtHandle h) {
  return "";
}

std::string R_IRInterface::toString(ExprHandle h) {
  return "";
}

std::string R_IRInterface::toString(OpHandle h) {
  return "";
}

std::string R_IRInterface::toString(MemRefHandle h) {
  return "";
}

std::string R_IRInterface::toString(SymHandle h) {
  return var_name(make_sexp(h));
}

std::string R_IRInterface::toString(ConstSymHandle h) {
  return "";
}

std::string R_IRInterface::toString(ConstValHandle h) {
  return "";
}

std::string R_IRInterface::toString(CallHandle h) {
  return "";
}

//--------------------------------------------------------------------
// R_RegionStmtIterator
//--------------------------------------------------------------------

// TODO: rename

StmtHandle R_RegionStmtIterator::current() const {
  return make_stmt_h(stmt_iter_ptr->current());
}

bool R_RegionStmtIterator::isValid() const { 
  return stmt_iter_ptr->isValid(); 
}

void R_RegionStmtIterator::operator++() {
  ++*stmt_iter_ptr;
}

void R_RegionStmtIterator::reset() {
  stmt_iter_ptr->reset();
}

/// Build the list of statements for iterator. Does not descend into
/// compound statements. The given StmtHandle is a CONS cell whose CAR
/// is the actual expression.
void R_RegionStmtIterator::build_stmt_list(StmtHandle stmt) {
  SEXP cell = make_sexp(stmt);
  SEXP exp = CAR(cell);
  switch (getSexpCfgType(exp)) {
  case CFG::SIMPLE:
    if (exp == R_NilValue) {
      stmt_iter_ptr = new R_ListIterator(exp);  // empty iterator
    } else {
      stmt_iter_ptr = new R_SingletonSEXPIterator(cell);
    }
    break;
  case CFG::COMPOUND:
    if (is_curly_list(exp)) {
      // Special case for a list of statments in curly braces.
      // CDR(exp), our list of statements, is not in a cell. But it is
      // guaranteed to be a list or nil, so we can call the iterator
      // that doesn't take a cell.
      stmt_iter_ptr = new R_ListIterator(CDR(exp));
    } else if (is_loop(exp)) {
      stmt_iter_ptr = new R_SingletonSEXPIterator(cell);
    } else {
      // We have a non-loop compound statement with a body. (body_c is
      // the cell containing the body.) This body might be a list for
      // which we're returning an iterator, or it might be some other
      // thing. First we figure out what the body is.
      SEXP body_c;
      if (is_fundef(exp)) {
	body_c = fundef_body_c(exp);
      } else if (is_paren_exp(exp)) {
	body_c = paren_body_c(exp);
      }
      // Now return the appropriate iterator.
      if (TYPEOF(CAR(body_c)) == NILSXP || TYPEOF(CAR(body_c)) == LISTSXP) {
	stmt_iter_ptr = new R_ListIterator(CAR(body_c));
      } else {
	stmt_iter_ptr = new R_SingletonSEXPIterator(body_c);
      }
    }
    break;
  default:
    stmt_iter_ptr = new R_SingletonSEXPIterator(cell);
    break;
  }
}

//--------------------------------------------------------------------
// R_RegionStmtListIterator
//--------------------------------------------------------------------

StmtHandle R_RegionStmtListIterator::current() const {
  return make_stmt_h(iter.current());
}

bool R_RegionStmtListIterator::isValid() const {
  return iter.isValid();
}

void R_RegionStmtListIterator::operator++() {
  ++iter;
}

void R_RegionStmtListIterator::reset() {
  iter.reset(); 
}

//--------------------------------------------------------------------
// R_UseDefAsLeafIterator
//--------------------------------------------------------------------

R_UseDefAsLeafIterator::R_UseDefAsLeafIterator(OA_ptr<R_VarRefSetIterator> iter)
  : m_iter(iter)
{
}

R_UseDefAsLeafIterator::~R_UseDefAsLeafIterator() {
}

LeafHandle R_UseDefAsLeafIterator::current() const {
  return make_leaf_h(m_iter->current()->get_sexp());
}
 
bool R_UseDefAsLeafIterator::isValid() {
  return m_iter->isValid();
}

void R_UseDefAsLeafIterator::operator++() {
  ++*m_iter;
}

void R_UseDefAsLeafIterator::reset() {
  m_iter->reset();
}

//------------------------------------------------------------
// R_UseDefAsMemRefIterator
//------------------------------------------------------------

R_UseDefAsMemRefIterator::R_UseDefAsMemRefIterator(OA_ptr<R_VarRefSetIterator> iter)
  : m_iter(iter)
{
}

R_UseDefAsMemRefIterator::~R_UseDefAsMemRefIterator() {
}

MemRefHandle R_UseDefAsMemRefIterator::current() const {
  return make_mem_ref_h(m_iter->current()->get_sexp());
}
 
bool R_UseDefAsMemRefIterator::isValid() const {
  return m_iter->isValid();
}

void R_UseDefAsMemRefIterator::operator++() {
  ++*m_iter;
}

void R_UseDefAsMemRefIterator::reset() {
  m_iter->reset();
}

//--------------------------------------------------------------------
// R_IRCallsiteIterator
//--------------------------------------------------------------------

R_IRCallsiteIterator::R_IRCallsiteIterator(StmtHandle _h)
  : m_annot(getProperty(ExpressionInfo, make_sexp(_h))),
    m_begin(m_annot->begin_call_sites()),
    m_end(m_annot->end_call_sites()),
    m_current(m_begin)
{
}

R_IRCallsiteIterator::~R_IRCallsiteIterator() {
}

CallHandle R_IRCallsiteIterator::current() const {
  return make_call_h(*m_current);
}

bool R_IRCallsiteIterator::isValid() const {
  return (m_current != m_end);
}

void R_IRCallsiteIterator::operator++() {
  ++m_current;
}

void R_IRCallsiteIterator::reset() {
  m_current = m_begin;
}


//----------------------------------------------------------------------
// R_ProcHandleIterator
//----------------------------------------------------------------------

// Basically a wrapper around FuncInfoIterator that fits the OA ProcHandleIterator interface

R_ProcHandleIterator::R_ProcHandleIterator(FuncInfo * fi) : m_fii(new FuncInfoIterator(fi)) {
}

R_ProcHandleIterator::~R_ProcHandleIterator() {
  delete m_fii;
}

ProcHandle R_ProcHandleIterator::current() const {
  return make_proc_h(m_fii->Current()->get_defn());
}

bool R_ProcHandleIterator::isValid() const {
  return m_fii->IsValid();
}

void R_ProcHandleIterator::operator++() {
  return ++(*m_fii);
}

void R_ProcHandleIterator::reset() {
  m_fii->Reset();
}

//------------------------------------------------------------
// R_CallMemRefHandleIterator
//------------------------------------------------------------

R_CallMemRefHandleIterator::R_CallMemRefHandleIterator(ExpressionInfo * stmt)
  : m_stmt(stmt), m_iter(stmt->begin_call_sites())
{
}

MemRefHandle R_CallMemRefHandleIterator::current() const {
  return make_mem_ref_h(*m_iter);
}

bool R_CallMemRefHandleIterator::isValid() const {
  return (m_iter != m_stmt->end_call_sites());
}

void R_CallMemRefHandleIterator::operator++() {
  ++m_iter;
}

void R_CallMemRefHandleIterator::reset() {
  m_iter = m_stmt->begin_call_sites();
}

//------------------------------------------------------------
// R_SingletonMemRefExprIterator
//------------------------------------------------------------

R_SingletonMemRefExprIterator::R_SingletonMemRefExprIterator(OA_ptr<MemRefExpr> mre)
  : R_SingletonIterator<OA_ptr<MemRefExpr> >(mre)
{
}

OA_ptr<MemRefExpr> R_SingletonMemRefExprIterator::current() const {
  return R_SingletonIterator<OA_ptr<MemRefExpr> >::current();
}

bool R_SingletonMemRefExprIterator::isValid() const {
  return R_SingletonIterator<OA_ptr<MemRefExpr> >::isValid();  
}

void R_SingletonMemRefExprIterator::operator++() {
  R_SingletonIterator<OA_ptr<MemRefExpr> >::operator++();
}

void R_SingletonMemRefExprIterator::reset() {
  R_SingletonIterator<OA_ptr<MemRefExpr> >::reset();
}
