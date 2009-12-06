// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: VFreshDFSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/Utils.h>
#include <analysis/VarRefFactory.h>

#include "VFreshDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef NameBoolDFSet MyDFSet;
typedef NameBoolDFSet::NameBoolDFSetIterator MyDFSetIterator;

static bool debug;

static std::pair<bool, OA_ptr<NameBoolDFSet> > make_pair(bool x, OA_ptr<NameBoolDFSet> y) {
  return std::pair<bool, OA_ptr<NameBoolDFSet> >(x, y);
}

VFreshDFSolver::VFreshDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_fact(VarRefFactory::get_instance())
{
  RCC_DEBUG("RCC_VFreshDFSolver", debug);
}

VFreshDFSolver::~VFreshDFSolver()
{}

OA_ptr<MyDFSet> VFreshDFSolver::perform_analysis(ProcHandle proc,
						   OA_ptr<CFG::CFGInterface> cfg)
{
  OA_ptr<MyDFSet> top; top = new MyDFSet();
  return perform_analysis(proc, cfg, top);
}

OA_ptr<MyDFSet> VFreshDFSolver::perform_analysis(ProcHandle proc,
						   OA_ptr<CFG::CFGInterface> cfg,
						   OA_ptr<MyDFSet> in_set)
{
  m_proc = proc;
  m_cfg = cfg;
  m_top = new MyDFSet();
  m_in = in_set;
  m_func_info = getProperty(FuncInfo, HandleInterface::make_sexp(proc));

  // use OA to solve data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Backward, *this);
  OA_ptr<DataFlow::DataFlowSet> entry_dfs = m_solver->solve(m_cfg, DataFlow::ITERATIVE);
  OA_ptr<MyDFSet> entry = entry_dfs.convert<MyDFSet>();
  return entry;
}

// ----- debugging -----

void VFreshDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void VFreshDFSolver::dump_node_maps(std::ostream &os) {
  OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA_ptr<MyDFSet> in_set, out_set;
  OA_ptr<CFG::NodesIteratorInterface> ni = m_cfg->getCFGNodesIterator();
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<CFG::NodeInterface> n = ni->current().convert<CFG::NodeInterface>();
    df_in_set = m_solver->getInSet(n);
    df_out_set = m_solver->getOutSet(n);
    in_set = df_in_set.convert<MyDFSet>();
    out_set = df_out_set.convert<MyDFSet>();
    os << "CFG NODE #" << n->getId() << ":\n";
    os << "IN SET:\n";
    in_set->dump(os, m_ir);
    os << "OUT SET:\n";
    out_set->dump(os, m_ir);
  }
}

// ----- callbacks for CFGDFProblem -----

OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::initializeTop() {
  Var * m;

  PROC_FOR_EACH_MENTION(m_func_info, m) {
    OA_ptr<MyDFSet::NameBoolPair> element;
    element = new MyDFSet::NameBoolPair(m_fact->make_body_var_ref((*m)->getMention_c()), false);
    m_top->insert(element);
  }
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::initializeBottom() {
  assert(0);
}

OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  if (n.ptrEqual(m_cfg->getExit())) {
    return m_in->clone();
  } else {
    return m_top->clone();
  }
}

/// CFGDFProblem says: OK to modify set1 and return it as result, because solver
/// only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						     OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  OA_ptr<MyDFSet> out; out = set1->meet(set2);
  return out.convert<DataFlow::DataFlowSet>();
}

/// CFGDFSolver says: OK to modify in set and return it again as
/// result because solver clones the BB in sets.
OA_ptr<DataFlow::DataFlowSet> VFreshDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_orig,
							 StmtHandle stmt)
{
  OA_ptr<MyDFSet> in; in = in_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> out;
  SEXP cell = make_sexp(stmt);
  out = vfresh(cell, in).second;
  return out.convert<DataFlow::DataFlowSet>();  // upcast
}

std::pair<bool, OA_ptr<NameBoolDFSet> > VFreshDFSolver::vfresh(SEXP cell, OA_ptr<NameBoolDFSet> c) {
  OA_ptr<NameBoolDFSet> s;
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_explicit_return(e)) {
    return vfresh(m_func_info->return_value_c(cell), c);
  } else if (is_fundef(e)) {
    return make_pair(false, c);
  } else if (is_symbol(e)) {
    return make_pair(true, c);
  } else if (is_const(e)) {
    return make_pair(false, c);
  } else if (is_struct_field(e)) {
    return make_pair(true, vfresh(struct_field_lhs_c(e), c).second);
  } else if (is_curly_list(e)) {
    return vfresh_curly_list(curly_body(e), c);
  } else if (is_call(e)) {
    // TODO
  } else if (is_assign(e) && is_struct_field(CAR(assign_lhs_c(e)))) {
    // TODO
  } else if (is_simple_assign(e) && is_local_assign(e)) {
    // TODO
  } else if (is_simple_assign(e) && is_free_assign(e)) {
    // TODO
  } else {
    assert(0);
  }
}

std::pair<bool, OA_ptr<NameBoolDFSet> > VFreshDFSolver::vfresh_curly_list(SEXP e, OA_ptr<NameBoolDFSet> c) {
}

OA_ptr<NameBoolDFSet> VFreshDFSolver::make_universal() {
  OA_ptr<NameBoolDFSet> all; all = m_top->clone().convert<NameBoolDFSet>();
  all->setUniversal();
  return all;
  
}
