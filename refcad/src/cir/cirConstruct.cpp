/****************************************************************************
  FileName     [ cirConstruct.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Functions for constructing circuit ]
  Author       [ music960633 ]
****************************************************************************/

#include <string>
#include <cassert>

#include "cir/cirMgr.h"


/**************************************
    Class CirMgr member functions
**************************************/
// add name-gate mapping
void CirMgr::addToMap(const std::string& name, CirGate* gate) {
  assert(name2GateMap_.find(name) == name2GateMap_.end());
  name2GateMap_[name] = gate;
}

// delete name from mapping
void CirMgr::delFromMap(const std::string& name) {
  assert(name2GateMap_.find(name) != name2GateMap_.end());
  name2GateMap_.erase(name);
}

// add gates
CirGate* CirMgr::addConstGate(const std::string& name) {
  if (const0_ != NULL) {
    if (const0_->getName() != "")
      delFromMap(const0_->getName());
    delete const0_;
  }
  const0_ = new CirConstGate(name);
  const0_->setId(0);
  if (name != "") addToMap(name, const0_);
  return const0_;
}

CirGate* CirMgr::addPiGate(const std::string& name) {
  CirGate* piGate = new CirPiGate(name);
  gateList_.push_back(piGate);
  piList_.push_back(piGate);
  if (name != "") addToMap(name, piGate);
  return piGate;
}

CirGate* CirMgr::addPoGate(const std::string& name) {
  CirGate* poGate = new CirPoGate(name);
  gateList_.push_back(poGate);
  poList_.push_back(poGate);
  return poGate;
}

CirGate* CirMgr::addAIGate(const std::string& name) {
  CirGate* aiGate = new CirAIGate(name);
  gateList_.push_back(aiGate);
  aigList_.push_back(aiGate);
  if (name != "") addToMap(name, aiGate);
  return aiGate;
}

// delete gates
void CirMgr::delAIGate(unsigned idx) {
  assert(idx < gateList_.size());
  CirGate* gate = gateList_[idx];
  assert(gate->getType() == GATE_AIG);
  assert(gate->getFanout().size() == 0);
  CirGateV in0 = gate->getFanin0();
  CirGateV in1 = gate->getFanin1();
  if (in0.gate() != NULL)
    in0.gate()->delFanout(CirGateV(gate, in0.isInv()));
  if (in1.gate() != NULL)
    in1.gate()->delFanout(CirGateV(gate, in1.isInv()));
  gateList_[idx] = gateList_.back();
  gateList_.pop_back();
  delete gate;
}

void CirMgr::connect1inputGate(const std::string& type,
                               CirGate* out, CirGate* in0) {
  if (type == "buf") {
    connectBuf(out, in0);
  } else if (type == "not") {
    connectNot(out, in0);
  } else {
    assert(0);
  }
}

void CirMgr::connect2inputGate(const std::string& type,
                               CirGate* out, CirGate* in0, CirGate* in1) {
  if (type == "and" ) {
    connectAnd (out, in0, in1);
  } else if (type == "nand") {
    connectNand(out, in0, in1);
  } else if (type == "or"  ) {
    connectOr  (out, in0, in1);
  } else if (type == "nor" ) {
    connectNor (out, in0, in1);
  } else if (type == "xor" ) {
    connectXor (out, in0, in1);
  } else if (type == "xnor") {
    connectXnor(out, in0, in1);
  } else {
    assert(0);
  }
}

// connect PO
void CirMgr::connectPo(CirGate* out, CirGate* in0) {
  out->setFanin(CirGateV(in0, false));
  in0->addFanout(CirGateV(out, false));
}

// connect AIG
void CirMgr::connectAIG(CirGate* out,
                        CirGate* in0, bool inv0,
                        CirGate* in1, bool inv1) {
  out->setFanin(CirGateV(in0, inv0), CirGateV(in1, inv1));
  in0->addFanout(CirGateV(out, inv0));
  in1->addFanout(CirGateV(out, inv1));
}

// connect logic gates
void CirMgr::connectBuf(CirGate* out, CirGate* in0) {
  connectAnd(out, in0, in0);
}

void CirMgr::connectNot(CirGate* out, CirGate* in0) {
  connectNor(out, in0, in0);
}

void CirMgr::connectAnd(CirGate* out, CirGate* in0, CirGate* in1) {
  connectAIG(out, in0, false, in1, false);
}

void CirMgr::connectNand(CirGate* out, CirGate* in0, CirGate* in1) {
  CirGate* tmp = addAIGate();
  connectNot(out, tmp);
  connectAnd(tmp, in0, in1);
}

void CirMgr::connectOr(CirGate* out, CirGate* in0, CirGate* in1) {
  CirGate* tmp = addAIGate();
  connectNot(out, tmp);
  connectNor(tmp, in0, in1);
}

void CirMgr::connectNor(CirGate* out, CirGate* in0, CirGate* in1) {
  connectAIG(out, in0, true, in1, true);
}

void CirMgr::connectXor(CirGate* out, CirGate* in0, CirGate* in1) {
  // a XOR b = ~(a*b + ~a*~b)
  CirGate* tmp1 = addAIGate();
  CirGate* tmp2 = addAIGate();
  connectNor(out, tmp1, tmp2);
  connectAnd(tmp1, in0, in1);
  connectNor(tmp2, in0, in1);
}

void CirMgr::connectXnor(CirGate* out, CirGate* in0, CirGate* in1) {
  CirGate* tmp = addAIGate();
  connectNot(out, tmp);
  connectXor(tmp, in0, in1);
}

void CirMgr::replaceGate(CirGate* orig, CirGateV repl) {
  // std::cerr << "Optimizing gate \"" << orig->getName() << "\"" << std::endl;
  const GateVList out = orig->getFanout();
  for (unsigned i = 0, n = out.size(); i < n; ++i) {
    CirGate* gate = out[i].gate();
    CirGateV in0 = gate->getFanin0();
    CirGateV in1 = gate->getFanin1();
    assert(in0.gate() == orig || in1.gate() == orig);
    if (in0.gate() == orig) {
      gate->setFanin((in0.isInv()? ~repl : repl), in1);
      repl.gate()->addFanout(repl.isInv()? ~out[i] : out[i]);
    } else {
      gate->setFanin(in0, (in1.isInv()? ~repl : repl));
      repl.gate()->addFanout(repl.isInv()? ~out[i] : out[i]);
    }
  }
  orig->clearFanout();
}

