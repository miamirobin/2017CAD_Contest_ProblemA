/****************************************************************************
  FileName     [ cirReport.cpp ]
  PackageName  [ cir ]
  Synopsis     [ report commands for CirMgr ]
  Author       [ music960633 ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>

#include "cir/cirMgr.h"

/**************************************
    Class CirMgr member functions
**************************************/
// report simple statistics
void CirMgr::report() const {
  unsigned numPi = piList_.size();
  unsigned numPo = poList_.size();
  unsigned numAig = aigList_.size();
  unsigned numTot = gateList_.size();
  assert(numPi + numPo + numAig == numTot);
  /*
  std::cout << "============================"  << std::endl;
  std::cout << "--- Reporting statictics ---"  << std::endl;
  std::cout << "============================"  << std::endl;
  std::cout << "PI   : " << numPi              << std::endl;
  std::cout << "PO   : " << numPo              << std::endl;
  std::cout << "AIG  : " << numAig             << std::endl;
  std::cout << "----------------------------"  << std::endl;
  std::cout << "Total: " << numTot             << std::endl;
  std::cout << std::endl;
  */
  std::cout << std::endl;
  std::cout << "Circuit Statistics" << std::endl;
  std::cout << "==================" << std::endl;
  std::cout << "  PI   " << std::setw(9) << std::right << numPi  << std::endl;
  std::cout << "  PO   " << std::setw(9) << std::right << numPo  << std::endl;
  std::cout << "  AIG  " << std::setw(9) << std::right << numAig << std::endl;
  std::cout << "------------------" << std::endl;
  std::cout << "  Total" << std::setw(9) << std::right << numTot << std::endl;
  std::cout << std::endl;
}

// report netlist
void CirMgr::reportNetlist() const {
  /*
  std::cout << "========================="  << std::endl;
  std::cout << "--- Reporting Netlist ---"  << std::endl;
  std::cout << "========================="  << std::endl;
  */
  CirGate::incFlg();
  unsigned id = 0;
  std::cout << std::endl;
  for (unsigned i = 0, n = poList_.size(); i < n; ++i)
    poList_[i]->reportNetlist(id);
  std::cout << std::endl;
}


// report Patch
void CirMgr::reportPatch() const{
  std::cout << "=========================" << std::endl;
  std::cout << "--- Reporting Patch ---" << std::endl;
  std::cout << "=========================" << std::endl;
  CirGate::incFlg();
  for (unsigned i = 0, n = tList_.size(); i < n; ++i){
   poList_[poList_.size()+i-tList_.size()]->reportpatch();
 }
  std::cout << std::endl;
}




// report all POs
void CirMgr::reportAllPo() const {
  std::cout << "=========================" << std::endl;
  std::cout << "--- Reporting All POs ---" << std::endl;
  std::cout << "=========================" << std::endl;
  CirGate::incFlg();
  for (unsigned i = 0, n = poList_.size(); i < n; ++i)
    poList_[i]->reportFanin();
  std::cout << std::endl;
}

// report AIG list
void CirMgr::reportAigList() const {
  std::cout << "==========================" << std::endl;
  std::cout << "--- Reporting AIG list ---" << std::endl;
  std::cout << "==========================" << std::endl;
  for (unsigned i = 0, n = aigList_.size(); i < n; ++i)
    aigList_[i]->report();
  std::cout << std::endl;
}

// report DFS list
void CirMgr::reportTopoList() const {
  std::cout << "==================================" << std::endl;
  std::cout << "--- Reporting topological list ---" << std::endl;
  std::cout << "==================================" << std::endl;
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i)
    topoList_[i]->report();
  std::cout << std::endl;
}

// report fanin cone size of POs
void CirMgr::reportPoFaninSize() const {
  std::cout << "===============================" << std::endl;
  std::cout << "--- Reporting PO fanin size ---" << std::endl;
  std::cout << "===============================" << std::endl;
  for (unsigned i = 0, n = poList_.size(); i < n; ++i)
    std::cout << getPoFaninSize(i) << " ";
  std::cout << std::endl;
}

// report fanout cone size of PIs
void CirMgr::reportPiFanoutSize() const {
  std::cout << "================================" << std::endl;
  std::cout << "--- Reporting PI fanout size ---" << std::endl;
  std::cout << "================================" << std::endl;
  for (unsigned i = 0, n = piList_.size(); i < n; ++i)
    std::cout << getPiFanoutSize(i) << " ";
  std::cout << std::endl;
}

// report FEC group list
void CirMgr::reportFecGrpList() const {
  std::cout << "================================" << std::endl;
  std::cout << "--- Reporting FEC group list ---" << std::endl;
  std::cout << "================================" << std::endl;
  for (unsigned i = 0, n = fecGrpList_.size(); i < n; ++i) {
    FecGrp* fecGrp = fecGrpList_[i];
    for (unsigned j = 0, m = fecGrp->size(); j < m; ++j) {
      CirGateV gateV = fecGrp->at(j);
      if (gateV.isInv()) std::cout << "!";
      std::cout << "[" << gateV.gate()->getName() << "] ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

// report unate array
void CirMgr::reportUnateArr() const {
  std::cout << "=============================" << std::endl;
  std::cout << "--- Reporting unate array ---" << std::endl;
  std::cout << "=============================" << std::endl;
  unsigned n = getPiNum();
  unsigned m = getPoNum();
  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = 0; j < m; ++j)
      std::cout << unateArr_[i][j].getVal() << " ";
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

// report EQ POs
void CirMgr::reportEqPo() const {
  std::cout << "========================" << std::endl;
  std::cout << "--- Reporting EQ POs ---" << std::endl;
  std::cout << "========================" << std::endl;
  std::vector<bool> visit(poList_.size(), false);
  for (unsigned i = 0, n = poList_.size(); i < n; ++i) {
    if (visit[i]) continue;
    visit[i] = true;
    for (unsigned j = i+1; j < n; ++j) {
      CirGateV gateV0 = poList_[i]->getFanin0();
      CirGateV gateV1 = poList_[j]->getFanin0();
      if (gateV0.gate() == gateV1.gate()) {
        if (gateV0.isInv() ^ gateV1.isInv()) {
          std::cout << getPoName(i) << " = !" << getPoName(j) << std::endl;
        } else {
          std::cout << getPoName(i) << " " << getPoName(j) << std::endl;
        }
        visit[j] = true;
      }
    }
  }
  std::cout << std::endl;
}
