/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Basic gate types ]
  Author       [ sam031023 ]
****************************************************************************/

#include "cir/cirGate.h"

#include <string>

#include "util/unate.h"

/**************************************
    Static variables and functions
**************************************/
static const CirGateV nullGateV = CirGateV(0, false, false);
static const CirGateV floatGateV = CirGateV(0, false, true);

unsigned CirGate::dfsFlg_static = 0;

/**************************************
  Class CirGate member functions
**************************************/
// Report gate
void CirGate::report(bool newline) const {
  const std::string gateTypeName[] = {"CONST", "PI", "PO", "AIG", "UNDEF"};
  std::cout << gateTypeName[getType()]
            << " (" << getName() << ") "<<getWeight()<<" [" << this << "]";
  if(newline)
    std::cout << std::endl;
}

// report netlist
void CirGate::reportNetlist(unsigned& id) const {
  if (isFlg()) return;
  setToFlg();
  if (getType() == GATE_CONST) {
    std::cout << "[" << id++ << "] " << "CONST0" << std::endl;
  } else if (getType() == GATE_PI) {
    std::cout << "[" << id++ << "] " << "PI  " << getName() << std::endl;
  } else if (getType() == GATE_PO) {
    in0_.gate()->reportNetlist(id);
    std::cout << "[" << id++ << "] " << "PO  " << getName() << " ";
    std::cout << (in0_.isInv() ? "!" : "") << in0_.gate()->getName() << std::endl;
  } else if (getType() == GATE_AIG) {
    in0_.gate()->reportNetlist(id);
    in1_.gate()->reportNetlist(id);
    std::cout << "[" << id++ << "] " << "AIG " << getName() << " ";
    std::cout << (in0_.isInv() ? "!" : "") << in0_.gate()->getName() << " ";
    std::cout << (in1_.isInv() ? "!" : "") << in1_.gate()->getName() << std::endl;
  }
}

// report fanin
void CirGate::reportFanin() const {
  reportFanin_rec(0, false);
}

// recursively report fanin
void CirGate::reportFanin_rec(unsigned indent, bool inv) const {
  for (unsigned i = 0; i < indent; ++i) std::cout << "  ";
  if (inv) std::cout << "!";
  report(false);
  if (isFlg()) {
   std::cout << "*" << std::endl;
   return;
 }
   else {
    setToFlg();
    std::cout << std::endl;
    if (!in0_.isNull()) {
      if (!in0_.isFloat())
        in0_.gate()->reportFanin_rec(indent+1, in0_.isInv());
      else
        std::cout << "Z" << std::endl;
    }
    if (!in1_.isNull()) {
      if(!in1_.isFloat())
        in1_.gate()->reportFanin_rec(indent+1, in1_.isInv());
      else
        std::cout << "Z" << std::endl;
    }
    
  }
}
/*
//output fanin
void CirGate::outputFanin() const {
  incFlg();
  outputFanin_rec(0, false);
}

// recursively output fanin
void CirGate::outputFanin_rec(unsigned indent, bool inv) const {
  //for (unsigned i = 0; i < indent; ++i) std::cout << "  ";
  if (inv) std::cout << "!";
  std::cout<<getName()<<" ";
  
  if (isFlg()) {
    std::cout << "*" << std::endl;
    return;
  } else {
    setToFlg();
    if (!in0_.isNull()) {
      if (!in0_.isFloat())
        in0_.gate()->outputFanin_rec(indent+1, in0_.isInv());
      else
        std::cout << "Z" << std::endl;
    }
    if (!in1_.isNull()) {
      if(!in1_.isFloat())
        in1_.gate()->outputFanin_rec(indent+1, in1_.isInv());
      else
        std::cout << "Z" << std::endl;
    }
    std::cout<<std::endl;
  }
}*/

// remove a fanout, returns true if succeed
bool CirGate::delFanout(CirGateV out) {
  for (unsigned i = 0, n = out_.size(); i < n; ++i) {
    if (out_[i] == out) {
      out_[i] = out_.back();
      out_.pop_back();
      return true;
    }
  }
  return false;
}

// get self's phase in FEC group
bool CirGate::getFecPhase() const {
  assert(fecGrp_ != NULL);
  for (unsigned i = 0, n = fecGrp_->size(); i < n; ++i) {
    if (fecGrp_->at(i).gate() == this)
      return fecGrp_->at(i).isInv();
  }
  assert(0);
  return false;
}

/**************************************
  Class CirConstGate member functions
**************************************/
CirConstGate::CirConstGate(std::string name):
  CirGate(name, nullGateV, nullGateV) {}

void CirConstGate::simulate() {
  simVal_ = 0;
}

void CirConstGate::simulate_rec() {
  setToFlg();
  simVal_ = 0;
}

void CirConstGate::simulate3v() {
  simVal3v_ = false_3v;
}

void CirConstGate::simUnate() {
  unateness_ = Unateness(0);
}

void CirConstGate::genCNF(SatSolver* solver) const {
  solver->addUnitCNF(getVar(), false);
}

void CirConstGate::setFanin3v() const {}

/**************************************
  Class CirPiGate member functions
**************************************/
CirPiGate::CirPiGate(std::string name):
  CirGate(name, nullGateV, nullGateV) {}

void CirPiGate::simulate() {}

void CirPiGate::simulate_rec() {}

void CirPiGate::simulate3v() {}

void CirPiGate::simUnate() {}

void CirPiGate::genCNF(SatSolver* solver) const {}

void CirPiGate::setFanin3v() const {}

/**************************************
  Class CirPoGate member functions
**************************************/
CirPoGate::CirPoGate(std::string name):
  CirGate(name, floatGateV, nullGateV) {}

CirPoGate::CirPoGate(std::string name, CirGateV in0):
  CirGate(name, in0, nullGateV) {}

void CirPoGate::simulate() {
  CirGate* g0 = in0_.gate();
  simVal_ = (in0_.isInv() ? ~(g0->getSimVal()) : g0->getSimVal());
}

void CirPoGate::simulate_rec() {
  setToFlg();
  CirGate* g0 = in0_.gate();
  if(!g0->isFlg()) g0->simulate_rec();
  simVal_ = (in0_.isInv() ? ~(g0->getSimVal()) : g0->getSimVal());
}

void CirPoGate::simulate3v() {
  CirGate* g0 = in0_.gate();
  simVal3v_ = (in0_.isInv() ? !(g0->getSimVal3v()) : g0->getSimVal3v());
}

void CirPoGate::simUnate() {
  CirGate* g0 = in0_.gate();
  unateness_ = (in0_.isInv() ? ~(g0->getUnateness()) : g0->getUnateness());
}

void CirPoGate::genCNF(SatSolver* solver) const {
  solver->addEqCNF(getVar(), in0_.gate()->getVar(), in0_.isInv());
}

void CirPoGate::setFanin3v() const {
  if(simVal3v_ != undef_3v)
    in0_.gate()->setSimVal3v(in0_.isInv()? !simVal3v_ : simVal3v_);
}

/**************************************
  Class CirAIGate member functions
**************************************/
CirAIGate::CirAIGate(std::string name):
  CirGate(name, floatGateV, floatGateV) {}

CirAIGate::CirAIGate(std::string name, CirGateV in0, CirGateV in1):
  CirGate(name, in0, in1) {}

void CirAIGate::simulate() {
  CirGate* g0 = in0_.gate();
  CirGate* g1 = in1_.gate();
  unsigned simVal0 = (in0_.isInv() ? ~(g0->getSimVal()) : g0->getSimVal());
  unsigned simVal1 = (in1_.isInv() ? ~(g1->getSimVal()) : g1->getSimVal());
  simVal_ = simVal0 & simVal1;
}

void CirAIGate::simulate_rec() {
  setToFlg();
  CirGate* g0 = in0_.gate();
  CirGate* g1 = in1_.gate();
  if (!g0->isFlg()) g0->simulate_rec();
  if (!g1->isFlg()) g1->simulate_rec();
  unsigned simVal0 = (in0_.isInv() ? ~(g0->getSimVal()) : g0->getSimVal());
  unsigned simVal1 = (in1_.isInv() ? ~(g1->getSimVal()) : g1->getSimVal());
  simVal_ = simVal0 & simVal1;
}

void CirAIGate::simulate3v() {
  CirGate* g0 = in0_.gate();
  CirGate* g1 = in1_.gate();
  bool_3v simVal0 = (in0_.isInv() ? !(g0->getSimVal3v()) : g0->getSimVal3v());
  bool_3v simVal1 = (in1_.isInv() ? !(g1->getSimVal3v()) : g1->getSimVal3v());
  simVal3v_ = simVal0 && simVal1;
}

void CirAIGate::simUnate() {
  CirGate* g0 = in0_.gate();
  CirGate* g1 = in1_.gate();
  Unateness u0 = (in0_.isInv() ? ~(g0->getUnateness()) : g0->getUnateness());
  Unateness u1 = (in1_.isInv() ? ~(g1->getUnateness()) : g1->getUnateness());
  unateness_ = u0 | u1;
}

void CirAIGate::genCNF(SatSolver* solver) const {
  solver->addAndCNF(getVar(),
                   in0_.gate()->getVar(), in0_.isInv(),
                   in1_.gate()->getVar(), in1_.isInv());
}

void CirAIGate::setFanin3v() const {
  if (simVal3v_ == true_3v) {
    in0_.gate()->setSimVal3v(in0_.isInv()? false_3v : true_3v);
    in1_.gate()->setSimVal3v(in1_.isInv()? false_3v : true_3v);
  } else if (simVal3v_ == false_3v) {
    if ((in0_.gate()->getSimVal() & 1) == (in0_.isInv()? 1 : 0)) {
      in0_.gate()->setSimVal3v(in0_.isInv()? true_3v : false_3v);
    } else if ((in1_.gate()->getSimVal() & 1) == (in1_.isInv()? 1 : 0)) {
      in1_.gate()->setSimVal3v(in1_.isInv()? true_3v : false_3v);
    } else {
      assert(0);
    }
  }
}
