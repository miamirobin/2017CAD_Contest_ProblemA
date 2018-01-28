/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Functions for gate simulation ]
  Author       [ music960633 ]
****************************************************************************/

#include "cirMgr.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>

/**************************************
    CirMgr member functions
**************************************/
// simulate the circuit
void CirMgr::simulate() {
  bool first = true;
  unsigned fail = 0;
  while (fail < 10) {
    for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
      unsigned simVal = 0;
      for (int k = 0; k < 32; ++k)
        simVal = (simVal << 1) + (rand() & 1);
      piList_[i]->setSimVal(simVal);
    }
    for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
      CirGate* gate = topoList_[i];
      gate->simulate();
    }
    if (first) {
      initFecGrpList();
      first = false;
    } else {
      if (!updateFecGrpList())
        fail += 1;
    }
  }
}

void CirMgr::simulate(const std::vector<unsigned>& pattern) {
  assert(pattern.size() == piList_.size());
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    piList_[i]->setSimVal(pattern[i]);
  }
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    gate->simulate();
  }
}

void CirMgr::simulate(const std::vector<bool>& pattern) {
  assert(pattern.size() == piList_.size());
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    piList_[i]->setSimVal(pattern[i]? ~0u : 0u);
  }
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    gate->simulate();
  }
}

void CirMgr::simulate3v(const std::vector<bool_3v>& pattern) {
  assert(pattern.size() == piList_.size());
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    piList_[i]->setSimVal3v(pattern[i]);
  }
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    gate->simulate3v();
  }
}

// expand cube with some don't cares
std::vector<bool_3v> CirMgr::expandCube(const std::vector<bool>& cube, int id) {
  unsigned nPi = piList_.size();
  assert(nPi == cube.size());
  simulate(cube);
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i)
    topoList_[i]->setSimVal3v(undef_3v);
  const0_->setSimVal3v(false_3v);
  poList_[id]->setSimVal3v(bool_3v(poList_[id]->getSimVal() & 1));
  for (unsigned i = topoList_.size()-1; i > 0; --i) {
    topoList_[i]->setFanin3v();
  }
  std::vector<bool_3v> ret(nPi);
  for (unsigned i = 0; i < nPi; ++i)
    ret[i] = piList_[i]->getSimVal3v();
  return ret;
}

// delete FEC groups
void CirMgr::delFecGrpList() {
  for(unsigned i = 0, n = fecGrpList_.size(); i < n; ++i)
    delete fecGrpList_[i];
  fecGrpList_.clear();
}

// Initialize FEC groups
// The circuit must be simulated before calling this function
void CirMgr::initFecGrpList() {
  delFecGrpList();
  std::map<unsigned, FecGrp*> mp;
  // push the gates into FEC groups
  for(unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    if (gate->getType() == GATE_PI || gate->getType() == GATE_PO) continue;
    unsigned simVal = gate->getSimVal();
    FecGrp* fecGrp;
    if (mp.find(simVal) != mp.end()) {
      // positive phase found in map
      fecGrp = mp[simVal];
      fecGrp->push_back(CirGateV(gate, false));
      gate->setFecGrp(fecGrp);
    } else if (mp.find(~simVal) != mp.end()) {
      // negative phase found in map
      fecGrp = mp[~simVal];
      fecGrp->push_back(CirGateV(gate, true));
      gate->setFecGrp(fecGrp);
    } else {
      // new FEC group
      fecGrp = new FecGrp;
      fecGrp->push_back(CirGateV(gate, false));
      gate->setFecGrp(fecGrp);
      mp[simVal] = fecGrp;
    }
  }
  collectFecGrp(mp, &fecGrpList_);
}

// update FEC groups
bool CirMgr::updateFecGrpList() {
  FecGrpList newFecGrpList;
  bool update = false;
  for (unsigned i = 0, n = fecGrpList_.size(); i < n; ++i) {
    update |= updateFecGrp(fecGrpList_[i], &newFecGrpList);
  }
  delFecGrpList();
  fecGrpList_ = newFecGrpList;
  return update;
}

// update one FEC group, returns false if nothing is updated
bool CirMgr::updateFecGrp(FecGrp* oldFecGrp, FecGrpList* fecGrpList) {
  std::map<unsigned, FecGrp*> mp;
  unsigned groupCnt = 0;
  for (unsigned i = 0, n = oldFecGrp->size(); i < n; ++i) {
    CirGateV gateV = oldFecGrp->at(i);
    CirGate* gate = gateV.gate();
    unsigned simVal = (gateV.isInv()? ~(gate->getSimVal()) : gate->getSimVal());
    FecGrp* fecGrp;
    if (mp.find(simVal) != mp.end()) {
      // simVal found in map
      fecGrp = mp[simVal];
      fecGrp->push_back(gateV);
      gate->setFecGrp(fecGrp);
    } else {
      // new FEC group
      fecGrp = new FecGrp;
      fecGrp->push_back(gateV);
      gate->setFecGrp(fecGrp);
      mp[simVal] = fecGrp;
      groupCnt += 1;
    }
  }
  collectFecGrp(mp, fecGrpList);
  return groupCnt != 1;
}

// collect groups with more than two members and push into fecGrpList
void CirMgr::collectFecGrp(const std::map<unsigned, FecGrp*>& mp,
                           FecGrpList* fecGrpList) {
  std::map<unsigned, FecGrp*>::const_iterator it;
  for (it = mp.begin(); it != mp.end(); ++it) {
    FecGrp* fecGrp = it->second;
    assert(fecGrp->size() > 0);
    if (fecGrp->size() == 1) {
      // only one gateV in group
      fecGrp->at(0).gate()->setFecGrp(NULL);
      delete fecGrp;
    } else {
      // otherwise, add to fecGrpList_
      fecGrpList->push_back(fecGrp);
    }
  }
}
