#include <algorithm>

#include <ParseInfo.h>

#include <support/RccError.h>

#include <analysis/AnalysisException.h>
#include <analysis/Annotation.h>
#include <analysis/Utils.h>
#include <analysis/SimpleIterators.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>

#include <analysis/LocalityDFSolver.h>
#include <analysis/ScopeTreeBuilder.h>
#include <analysis/LocalVariableAnalysis.h>
#include <analysis/LocalFunctionAnalysis.h>
// #include <analysis/BindingAnalysis.h>
#include <analysis/CallGraphBuilder.h>

#include "Analyst.h"

using namespace RAnnot;
using namespace RProp;

// ----- implement Singleton pattern -----

// the only instance
R_Analyst * R_Analyst::m_instance = 0;

// static instantiation
R_Analyst * R_Analyst::get_instance(SEXP _program) {
  if (m_instance == 0) {
    m_instance = new R_Analyst(_program);
  }
  return m_instance;
}

// just get the existing instance; error if not yet instantiated
R_Analyst * R_Analyst::get_instance() {
  if (m_instance == 0) {
    rcc_error("R_Analyst is not yet instantiated");
  }
  return m_instance;
}

// ----- constructor -----

//! construct an R_Analyst by providing an SEXP representing the whole program
R_Analyst::R_Analyst(SEXP _program) : m_program(_program)
  {}

// ----- analysis -----

bool R_Analyst::perform_analysis() {
  try {
    m_interface = new R_IRInterface();
    m_scope_tree_root = ScopeTreeBuilder::build_scope_tree_with_given_root(m_program);
    if (ParseInfo::allow_oo()           ||
	ParseInfo::allow_envir_manip()  ||
	ParseInfo::allow_special_redef())
    {
      throw AnalysisException();
    }
    build_cfgs();
    //    build_local_variable_info();
    build_local_function_info();
    //    build_locality_info();
    build_bindings();
    build_call_graph();
    return true;
  }
  catch (AnalysisException ae) {
    // One phase of analysis rejected a program. Get rid of the
    // information in preparation to compile trivially.
    rcc_warn("analysis encountered problems; compiling trivially");
    clearProperties();
    return false;
  }
}

FuncInfo *R_Analyst::get_scope_tree_root() {
  return m_scope_tree_root;
}

//! Dump the CFG of the given function definition. Located here in
//! R_Analyst because cfg->dump needs the IRInterface as an argument.
void R_Analyst::dump_cfg(std::ostream &os, SEXP proc) {
  if (proc != R_NilValue) {
    FuncInfo *fi = getProperty(FuncInfo, proc);
    OA::OA_ptr<OA::CFG::Interface> cfg;
    cfg = fi->getCFG();
    cfg->dump(os, m_interface);
  }
}

void R_Analyst::dump_all_cfgs(std::ostream &os) {
  FuncInfoIterator fii(m_scope_tree_root);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo *finfo = fii.Current();
    OA::OA_ptr<OA::CFG::Interface> cfg;
    cfg = finfo->getCFG();
    cfg->dump(os, m_interface);
  }
}

//! Populate m_cfgs with the CFG for each procedure
void R_Analyst::build_cfgs() {
  FuncInfo *finfo = get_scope_tree_root();
  OA::CFG::ManagerStandard cfg_man(m_interface, true); // build statement-level CFGs

  // preorder traversal of scope tree
  FuncInfoIterator fii(finfo);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo *finfo = fii.Current();
    SEXP fundef = finfo->getDefn();
    OA::ProcHandle ph = HandleInterface::make_proc_h(fundef);
    OA::OA_ptr<OA::CFG::CFGStandard> cfg_ptr; cfg_ptr = cfg_man.performAnalysis(ph);
    finfo->setCFG(cfg_ptr);
  }
}

//! Collect basic local info on variables: use/def, "<-"/"<<-", etc.
void R_Analyst::build_local_variable_info() {
  // each function
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    OA::OA_ptr<OA::CFG::Interface> cfg; cfg = fi->getCFG();

    // each node in function's CFG
    OA::OA_ptr<OA::CFG::Interface::NodesIterator> ni = cfg->getNodesIterator();
    for ( ; ni->isValid(); ++*ni) {

      // each statement in node
      OA::OA_ptr<OA::CFG::Interface::NodeStatementsIterator> si;
      si = ni->current()->getNodeStatementsIterator();
      for( ; si->isValid(); ++*si) {
	OA::StmtHandle stmt = si->current();
	LocalVariableAnalysis lva((SEXP)stmt.hval());
	lva.perform_analysis();
      }
    }
  }
}

//! Discovers local information on procedures: arguments, names
//! mentioned, etc.
void R_Analyst::build_local_function_info() {
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    LocalFunctionAnalysis lfa(fi->getDefn());
    lfa.perform_analysis();
  }
}

//! For each procedure, use control flow to discover locality for each
//! name (whether local or free)
void R_Analyst::build_locality_info() {
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    Locality::LocalityDFSolver uds(m_interface);
    OA::ProcHandle ph = HandleInterface::make_proc_h(fi->getDefn());
    uds.perform_analysis(ph, fi->getCFG());
  }
}

//! Perform binding analysis to resolve names to a single scope if
//! possible
void R_Analyst::build_bindings() {
  //  BindingAnalysis ba(m_scope_tree_root);
  //  ba.perform_analysis();
}

//! For each procedure, discover which other procedures are called
void R_Analyst::build_call_graph() {
  CallGraphBuilder cgb(m_scope_tree_root);
  cgb.perform_analysis();
}
