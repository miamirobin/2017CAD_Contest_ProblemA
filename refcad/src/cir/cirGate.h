/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define classes for basic gate types ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CIR_CIRGATE_H_
#define CIR_CIRGATE_H_

#include <string>

#include "cir/cirDef.h"
#include "minisat/sat.h"
#include "util/bool3v.h"
#include "util/unate.h"

/**************************************
  Forward declaration
**************************************/
class CirGate;

/**************************************
  Enum GateType definition
**************************************/
enum GateType {
  GATE_CONST,
  GATE_PI,
  GATE_PO,
  GATE_AIG,

  GATE_UNDEF
};

/**************************************
  Class CirGateV definition
**************************************/
// Represents a wire connection with bubble
// -- gate() returns the child gate pointer (CirGate*)
// -- isInv() be true if the signal is inversed
// -- isFloat() be true if the wire is floating
// -- isNull() be true if the gate does not actually have the wire
// -- -- e.g. A primary input gate does not have any inputs
class CirGateV {
 public:
  explicit CirGateV(CirGate* gate = 0, bool inv = false, bool flt = false):
    gateV_(reinterpret_cast<uintptr_t>(gate)) {
    if(inv) setInv();
    if(flt) setFloat();
  }
  CirGateV(const CirGateV& v) { gateV_ = v.gateV_; }
  CirGateV operator = (const CirGateV& v) {
    gateV_ = v.gateV_;
    return *this;
  }
  ~CirGateV() {}

  bool operator <  (const CirGateV& v) const { return gateV_ <  v.gateV_; }
  bool operator >  (const CirGateV& v) const { return gateV_ >  v.gateV_; }
  bool operator <= (const CirGateV& v) const { return gateV_ <= v.gateV_; }
  bool operator >= (const CirGateV& v) const { return gateV_ >= v.gateV_; }
  bool operator == (const CirGateV& v) const { return gateV_ == v.gateV_; }
  bool operator != (const CirGateV& v) const { return gateV_ != v.gateV_; }
  CirGateV operator ~() const { return CirGateV(gate(), !isInv(), isFloat()); }

  CirGate* gate() const {
    return reinterpret_cast<CirGate*>(gateV_ & PTR_MASK);
  }

  bool isInv()    const { return gateV_ & INV_MASK; }
  bool isFloat()  const { return gateV_ & FLT_MASK; }
  bool isNull()   const { return !isFloat() && (gate() == 0); }
  void setInv()   { gateV_ |= INV_MASK; }
  void setFloat() { gateV_ |= FLT_MASK; }
  void flipInv()  { gateV_ ^= INV_MASK; }

 private:
  uintptr_t gateV_;
  static const uintptr_t INV_MASK = 0x1;
  static const uintptr_t FLT_MASK = 0x2;
  static const uintptr_t PTR_MASK = ((~uintptr_t(0)) >> 2 << 2);
};


/**************************************
  Class CirGate definition
**************************************/
// Base class for all gate types
class CirGate {
 public:
  CirGate(std::string name, CirGateV in0, CirGateV in1):
    name_     (name),
    id_       (0),
    in0_      (in0),
    in1_      (in1),
    used_     (true),
    fecGrp_   (NULL),
    var_      (0),
    eqGate_   (CirGateV(0)),
    dfsFlg_   (0) {}
  virtual ~CirGate() {}

  // functions for DFS
  static void incFlg() { dfsFlg_static += 1; }
  bool isFlg() const { return dfsFlg_ == dfsFlg_static; }
  void setToFlg() const { dfsFlg_ = dfsFlg_static; }

  // functions for reporting
  void report(bool newline = true) const;
  void reportNetlist(unsigned& id) const;
  void reportFanin() const;

  // functions for getting data
  std::string       getName()       const { return name_; }
  unsigned          getId()         const { return id_; }
  bool              getUsed()       const { return used_; }
  CirGateV          getFanin0()     const { return in0_; }
  CirGateV          getFanin1()     const { return in1_; }
  const GateVList&  getFanout()     const { return out_; }
  virtual GateType  getType()       const { return GATE_UNDEF; }
  unsigned          getSimVal()     const { return simVal_; }
  bool_3v           getSimVal3v()   const { return simVal3v_; }
  FecGrp*           getFecGrp()     const { return fecGrp_; }
  bool              getFecPhase()   const;
  Var               getVar()        const { return var_; }
  CirGateV          getEqGate()     const { return eqGate_; }
  Unateness         getUnateness()  const { return unateness_; }

  // functions for getting data
  void setId(unsigned id) { id_ = id; }
  void setUsed(bool used) { used_ = used; }
  void setFanin(CirGateV in) { in0_ = in; }
  void setFanin(CirGateV in0, CirGateV in1) { in0_ = in0; in1_ = in1; }
  void addFanout(CirGateV out) { out_.push_back(out); }
  bool delFanout(CirGateV out);
  void clearFanout() { out_.clear(); }
  void setSimVal(unsigned simVal) { simVal_ = simVal; }
  void setSimVal3v(bool_3v simVal3v) { simVal3v_ = simVal3v; }
  void setFecGrp(FecGrp* fecGrp) { fecGrp_ = fecGrp; }
  void setVar(Var var) { var_ = var; }
  void setEqGate(CirGateV gate) { eqGate_ = gate; }
  void setUnateness(Unateness u) { unateness_ = u; }

  // functions for simulation
  virtual void simulate() = 0;
  virtual void simulate_rec() = 0;
  virtual void simulate3v() = 0;
  virtual void simUnate() = 0;

  // functions for SAT
  virtual void genCNF(SatSolver* solver) const = 0;

  // functions for 3v generalization
  virtual void setFanin3v() const = 0;

 protected:
  void reportFanin_rec(unsigned indent, bool inv) const;

  std::string       name_;
  unsigned          id_;
  CirGateV          in0_;
  CirGateV          in1_;
  GateVList         out_;
  bool              used_;
  unsigned          simVal_;
  bool_3v           simVal3v_;
  FecGrp*           fecGrp_;
  Var               var_;
  CirGateV          eqGate_;
  Unateness         unateness_;

  mutable unsigned  dfsFlg_;

  // static DFS flag
  static unsigned   dfsFlg_static;
};

/**************************************
  Class CirConstGate definition
**************************************/
// Constant 0 gate
class CirConstGate: public CirGate {
 public:
  explicit CirConstGate(std::string name);
  ~CirConstGate() {}

  GateType getType() const { return GATE_CONST; }

  void simulate();
  void simulate_rec();
  void simulate3v();
  void simUnate();
  void genCNF(SatSolver* solver) const;
  void setFanin3v() const;
};

/**************************************
  Class CirPiGate definition
**************************************/
// Primary input gate
class CirPiGate: public CirGate {
 public:
  explicit CirPiGate(std::string name);
  ~CirPiGate() {}

  GateType getType() const { return GATE_PI; }

  void simulate();
  void simulate_rec();
  void simulate3v();
  void simUnate();
  void genCNF(SatSolver* solver) const;
  void setFanin3v() const;
};

/**************************************
  Class CirPoGate definition
 **************************************/
// Primary output gate
class CirPoGate: public CirGate {
 public:
  explicit CirPoGate(std::string name);
  CirPoGate(std::string name, CirGateV in0);
  ~CirPoGate() {}

  GateType getType() const { return GATE_PO; }

  void simulate();
  void simulate_rec();
  void simulate3v();
  void simUnate();
  void genCNF(SatSolver* solver) const;
  void setFanin3v() const;
};

/**************************************
  Class CirAIGate definition
 **************************************/
// And-inverter gate
class CirAIGate: public CirGate {
 public:
  explicit CirAIGate(std::string name);
  CirAIGate(std::string name, CirGateV in0, CirGateV in1);
  ~CirAIGate() {}

  GateType getType() const { return GATE_AIG; }

  void simulate();
  void simulate_rec();
  void simulate3v();
  void simUnate();
  void genCNF(SatSolver* solver) const;
  void setFanin3v() const;
};

#endif  // CIR_CIRGATE_H_
