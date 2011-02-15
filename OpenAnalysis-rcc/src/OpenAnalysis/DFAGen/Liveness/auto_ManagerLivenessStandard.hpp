
/* ManagerAutoLivenessStandard.hpp
   NOTE: This file was automatically generated by DFAGen.  It is the interface
         for the Liveness analysis manager.
*/

#ifndef ManagerAutoLivenessStandard_H
#define ManagerAutoLivenessStandard_H

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/LivenessIRInterface.hpp>
#include <OpenAnalysis/DFAGen/Liveness/auto_LivenessStandard.hpp>

#include <OpenAnalysis/Alias/Interface.hpp>

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace Liveness {


class ManagerLivenessStandard
    : public virtual DataFlow::CFGDFProblem
{
  public:
    ManagerLivenessStandard(OA_ptr<LivenessIRInterface> _ir);
    ~ManagerLivenessStandard () {}

    OA_ptr<LivenessStandard> performAnalysis(
        ProcHandle proc,
        OA_ptr<CFG::CFGInterface> cfg,
        OA_ptr<Alias::Interface> alias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE);

  private:
    OA_ptr<DataFlow::DataFlowSet> initializeTop();
    OA_ptr<DataFlow::DataFlowSet> initializeBottom();

    OA_ptr<DataFlow::DataFlowSet>
       initializeNodeIN(OA_ptr<CFG::NodeInterface> n);

    OA_ptr<DataFlow::DataFlowSet>
       initializeNodeOUT(OA_ptr<CFG::NodeInterface> n);

    void dumpset(OA_ptr<LivenessDFSet> inSet);

    OA_ptr<DataFlow::DataFlowSet> meet(
        OA_ptr<DataFlow::DataFlowSet> set1,
        OA_ptr<DataFlow::DataFlowSet> set2);

    OA_ptr<DataFlow::DataFlowSet> genSet(StmtHandle stmt);

    OA_ptr<DataFlow::DataFlowSet> killSet(StmtHandle stmt,
        OA_ptr<DataFlow::DataFlowSet> X);

    OA_ptr<DataFlow::DataFlowSet> transfer(
        OA_ptr<DataFlow::DataFlowSet> X,
        OA::StmtHandle Stmt);

    OA_ptr<LivenessIRInterface> mIR;
    OA_ptr<Alias::Interface> mAlias;
    OA_ptr<LivenessStandard> mLivenessMap;
    OA_ptr<DataFlow::CFGDFSolver> mSolver;

    std::map<StmtHandle, set<OA_ptr<Location> > > mStmt2MayDefMap;
    std::map<StmtHandle, set<OA_ptr<Location> > > mStmt2MustDefMap;
    std::map<StmtHandle, set<OA_ptr<Location> > > mStmt2MayUseMap;
    std::map<StmtHandle, set<OA_ptr<Location> > > mStmt2MustUseMap;
};

  } // end of Liveness namespace
} // end of OA namespace

#endif
