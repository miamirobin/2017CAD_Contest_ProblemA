
/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Functions for fraig ]
  Author       [ sam031023 ]
****************************************************************************/

#include "cirMgr.h"

#include <cassert>

#include "minisat/satt.h"

/**************************************
    CirMgr member functions
**************************************/
// simulate the circuit
void CirMgr::fraig(int effort) {
  SatSolver solver;
  initSolver(&solver);
  unsigned orinum=oritopoList_.size();
  for (unsigned i = orinum, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    if (gate->getType() != GATE_AIG) continue;
    FecGrp* fecGrp = gate->getFecGrp();
    if (fecGrp == NULL) continue;
    assert(fecGrp->size() >= 2);
    CirGateV candidateV = fecGrp->at(0);
    CirGateV gateV = CirGateV(gate, gate->getFecPhase());
    lbool equal;
    if (gateV == candidateV) continue;
    if (candidateV.gate() == const0_) {
      assert(!candidateV.isInv());
      equal = checkConst0(&solver, gateV, effort);
    } else {
      equal = checkEquivalence(&solver, gateV, candidateV, effort);
    }
    if (equal == l_True) {
      gate->setEqGate(gateV.isInv() ? ~candidateV : candidateV);
      solver.addEqCNF(gateV.gate()->getVar(), candidateV.gate()->getVar(),
                      gateV.isInv() ^ candidateV.isInv());
    } else if (equal == l_False) {
      updateBySAT(&solver);
    }
  }
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    if (gate->getEqGate().gate() != 0) {
      replaceGate(gate, gate->getEqGate());
    }
  }
  buildAigList();
  buildTopoList();
  strash();
}

// initialize SAT solver
void CirMgr::initSolver(SatSolver* solver) {
  solver->init();
  addCircuitCNF(solver);
}

// add CNF of the circuit to SAT solver
void CirMgr::addCircuitCNF(SatSolver* solver) {                      
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
   
    CirGate* gate = topoList_[i];
    gate->setVar(solver->newVar());
    gate->genCNF(solver);
  }
}


// add CNF of the specific PO to SAT solver
void CirMgr::addPoCNF(SatSolver* solver) {   
  buildSelectPoTopoList();                   
  for (unsigned i = 0, n = SelectPotopoList_.size(); i < n; ++i) {
    
    CirGate* gate = SelectPotopoList_[i];
   
    gate->setVar(solver->newVar());
    gate->genCNF(solver);
   
  }
}



// check if a gate is equivalent to const 0
lbool CirMgr::checkConst0(SatSolver* solver, CirGateV gateV, int effort) const {
  if (effort == -1) {
    return lbool(!solver->solve(gateV.gate()->getVar(), !gateV.isInv()));
  } else {
    return !solver->solveLimited(gateV.gate()->getVar(), !gateV.isInv(), effort * 10000);
  }

}

// check if two gates are equivalent
lbool CirMgr::checkEquivalence(SatSolver* solver,
                               CirGateV gateV0, CirGateV gateV1, int effort) const {
  Var var = solver->newVar();
  solver->addXorCNF(var, gateV0.gate()->getVar(), gateV0.isInv(),
                    gateV1.gate()->getVar(), gateV1.isInv());
  if (effort == -1) {
    return lbool(!solver->solve(var, true));
  } else {
    return !solver->solveLimited(var, true, effort * 10000);
  }
}

// update FEC group by SAT
void CirMgr::updateBySAT(SatSolver* solver) {
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    CirGate* gate = piList_[i];
    gate->setSimVal(solver->getVal(gate->getVar()) ? 0xffffffff : 0x0);
  }
  for (unsigned i = 1, n = piList_.size(); i < 32; ++i) {
    unsigned invId = rand() % n;
    CirGate* gate = piList_[invId];
    gate->setSimVal(gate->getSimVal() ^ (1 << i));
  }
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    topoList_[i]->simulate();
  }
  updateFecGrpList();
}
