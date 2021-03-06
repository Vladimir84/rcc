/* ManagerAutoVary.hpp
   NOTE: This file was automatically generated by DFAGen.  It is the interface
         for the Vary analysis manager.
*/

#ifndef ManagerAutoVary_H
#define ManagerAutoVary_H

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/auto_VaryIRInterface.hpp>
#include <OpenAnalysis/DFAGen/Vary/auto_Vary.hpp>

#include <OpenAnalysis/Alias/Interface.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace Vary {


class ManagerVary
    : public virtual DataFlow::CFGDFProblem
{
  public:
    //*****************************************************************
    // Constructor / Destructor
    //*****************************************************************
    //! Construct new object to perform Vary analysis
    ManagerVary(OA_ptr<VaryIRInterface> _ir);

    //! Destruct this object
    ~ManagerVary() {}


    //*****************************************************************
    // Analysis
    //*****************************************************************
    /*! Perform Vary analysis on the specified procedure given
        a control-flow graph of the procedure, alias results, and
        inter-procedural side-effect analysis results. */
    OA_ptr<Vary> performAnalysis(
        ProcHandle proc,
        OA_ptr<CFG::CFGInterface> cfg,
        OA_ptr<Alias::Interface> alias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE);

  private:
    //*****************************************************************
    // Methods to satisfy CFGDFProblem interface
    //*****************************************************************
    OA_ptr<DataFlow::DataFlowSet> initializeTop();

    OA_ptr<DataFlow::DataFlowSet> initializeBottom();

    OA_ptr<DataFlow::DataFlowSet>
       initializeNodeIN(OA_ptr<CFG::NodeInterface> n);

    OA_ptr<DataFlow::DataFlowSet>
       initializeNodeOUT(OA_ptr<CFG::NodeInterface> n);

    OA_ptr<DataFlow::DataFlowSet> meet(
        OA_ptr<DataFlow::DataFlowSet> set1,
        OA_ptr<DataFlow::DataFlowSet> set2);

    OA_ptr<DataFlow::DataFlowSet> transfer(
        OA_ptr<DataFlow::DataFlowSet> X,
        OA::StmtHandle Stmt);


    //*****************************************************************
    // Data-flow Equations
    //*****************************************************************
    // Calculate GEN set for specified statement
    OA_ptr<DataFlow::DataFlowSet> genSet(StmtHandle stmt,
        OA_ptr<DataFlow::DataFlowSet> X);

    // Calculate KILL set for specified statement
    OA_ptr<DataFlow::DataFlowSet> killSet(StmtHandle stmt,
        OA_ptr<DataFlow::DataFlowSet> X);


    //*****************************************************************
    // Debugging Methods
    //*****************************************************************
    void dumpset(OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag > > inSet);



    OA_ptr<VaryIRInterface> mIR;
    OA_ptr<Alias::Interface> mAlias;
    OA_ptr<Vary> mVaryMap;
    OA_ptr<DataFlow::CFGDFSolver> mSolver;

    // Maps for predefined-sets:
    std::map<StmtHandle, set<Alias::AliasTag > > mStmt2MayDefMap;
    std::map<StmtHandle, set<Alias::AliasTag > > mStmt2MustDefMap;
    std::map<StmtHandle, set<Alias::AliasTag > > mStmt2MayUseMap;
    std::map<StmtHandle, set<Alias::AliasTag > > mStmt2MustUseMap;
};

  } // end of Vary namespace
} // end of OA namespace

#endif
