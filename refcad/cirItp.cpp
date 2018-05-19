
#include "cir/cirMiter.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <cassert>




SatSolver solver;
initSolver(&solver);


// initialize SAT solver
void CirMiter::initSolver(SatSolver* solver) {
  solver->init();
  addCircuitCNF(solver);
}

// add CNF of the circuit to SAT solver
void CirMiter::addCircuitCNF(SatSolver* solver) {
  for (unsigned i = 0, n = ckt0->topolist().size(); i < n; ++i) {
    CirGate* gate = ckt0->topolist()[i];
    gate->setVar(solver->newVar());
    gate->genCNF(solver);
  }
}

candidateV.gate() = const0_;


/* ---  FUNCTION: itp_iterative-------------------------*//**

   Iteratively solve: Main Algorithm

 1. Use "itp_selNode" to select a node from _nodeSorted
 2. Use "itp_setType" to set up types of A and B
 3. Use "itp_assNode" to assign const value to the selected node
 4. Use "itp_common"  to build common-variable net to _onoff and set global types
 5. Use "itp_recover" to recover the net which is modified by (3) and (4) to original net
 6. Use "itp_getItp"  to get an interpolant if UNSAT
 7. Use "itp_replace" to put the generated interpolant into _onSet and _offSet
 8. Use "itp_judge"   to judge the selected node is candidate or not(complete)
 9. Use "itp_judgeSim"to judge the selected node is candidate or not by simulation(not completely judge)
 10.Use "itp_dominator" to compute dominators of each nodes(fast prune non-candidates)
 11.Use "itp_lockPO"  to lock the fixed POs by SAT
 12.Use "itp_final_fix" to fix unfixed POs at last

   _mode:
      ONE_PO      : fix at least one PO of a candidate in its fanout cone
      FANOUT_POS  : fix at all POs of a candidate in its fanout cone
      BOTH        : Mix the above two mode

   _poDone:
      All elements are 1: ECO Done
      Else              : Iterative ECO process

   this_po_giveup : if true, fix this PO at itp_final_fix

\n*///-----------------------------------------------
/* ---  FUNCTION: itp_off-------------------------*//**
Interpolation Off set
\n*///----------------------------------------------------


CirGate* gate = aigList_[i];
assert(gate->getType() == GATE_AIG);
CirGateV in0 = gate->getFanin0();
CirGateV in1 = gate->getFanin1();



void
IntEco::itp_off( AigNet& orig ){
   foreach( AigNode& n, orig._poList )
      _poSet.push_back(n);
   vector<AigNode> limit = itp_poLimit( orig, OFFSET );
   foreach ( AigNode& n, limit )
      _poLimit.push_back(n);
   AigNode offsetOut = itp_poDynamicLimit( limit, 0);
   buildNet( *_offSet, offsetOut, true );
   _offSet->_poList.push_back(offsetOut);
}

/* ---  FUNCTION: itp_on-------------------------*//**
Interpolation On set
\n*///----------------------------------------------------
void
IntEco::itp_on( AigNet& orig ){
   foreach( AigNode& n, orig._poList )
      _poSet.push_back(n);
   vector<AigNode> limit = itp_poLimit( orig, ONSET );
   foreach ( AigNode& n, limit )
      _poLimit.push_back(n);
   AigNode onsetOut = itp_poDynamicLimit( limit, 1);
   buildNet( *_onSet, onsetOut, true );
   _onSet->_poList.push_back(onsetOut);
}

