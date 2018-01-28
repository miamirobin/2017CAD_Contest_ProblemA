/****************************************************************************
  FileName     [ cirOpt.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Functions for gate optimizing ]
  Author       [ music960633 ]
****************************************************************************/

#include "cirMgr.h"

#include <map>
#include <utility>
#include <cassert>

/**************************************
    CirMgr member functions
**************************************/
// fully optimize
void CirMgr::optimize_full() {
 
  sweep();
  optimize();       
  strash();             
  simulate();            
  fraig();              
  
}


void CirMgr::optimize_half() {
 
  sweep();
  optimize();       
  strash();             
            
  
}



unsigned CirMgr::SWeep() {
  unsigned removeCount = 0;
  bool flg = true;
  while (flg) {
    flg = false;
    for (unsigned i = 0; i < gateList_.size(); ++i) {
      CirGate* gate = gateList_[i];
      assert(gate != NULL);
      if (gate->getType() != GATE_AIG) continue;
      if (gate->getName() == "1'b1") continue;
      if (gate->getFanout().size() == 0) {
        // std::cerr << "removing \"" << gate->getName() << "\"" << std::endl;
      /*  delAIGate(i);
        --i;
        removeCount += 1;*/
        CirGate* poGate=addPoGate(gate->getName());
        connectPo(poGate, gate);
        flg = true;
      }
    }
  }
  buildAigList();
  buildTopoList();
  return removeCount;
}

// remove unused gates
unsigned CirMgr::sweep() {
  unsigned removeCount = 0;
  bool flg = true;
  while (flg) {
    flg = false;
    for (unsigned i = 0; i < gateList_.size(); ++i) {
      CirGate* gate = gateList_[i];
      assert(gate != NULL);
      if (gate->getType() != GATE_AIG) continue;
      if (gate->getFanout().size() == 0 and gate->getnoinput()) {
        // std::cerr << "removing \"" << gate->getName() << "\"" << std::endl;
        delAIGate(i);
        --i;
        removeCount += 1;
        flg = true;
      }
    }
  }
  buildAigList();
  buildTopoList();
  return removeCount;
}

// do trivial optimization
unsigned CirMgr::optimize() {
  const CirGateV const0V = CirGateV(const0_);
  
  for (unsigned i = 0, n = aigList_.size(); i < n; ++i) {  
    CirGate* gate = aigList_[i];        
   // if (!gate->getSelect()){continue;}
    assert(gate->getType() == GATE_AIG);               
    CirGateV in0 = gate->getFanin0();
    CirGateV in1 = gate->getFanin1();             
    //Case 1: one of fanin is constant 1
    //         -> replace by onother fanin
    if (in0.gate() == const0_ && in0.isInv())
      replaceGate(gate, in1);
    else if (in1.gate() == const0_ && in1.isInv())
      replaceGate(gate, in0);
    // Case 2: one of fanin is constant 0
    //         -> replace by const0_
    else if (in0.gate() == const0_ && !in0.isInv())
      replaceGate(gate, const0V);
    else if (in1.gate() == const0_ && !in1.isInv())
      replaceGate(gate, const0V);
    // Case 3: two fanins are the same
    //         -> replace by the fanin
    else if (in0 == in1)
      replaceGate(gate, in0);
    // Case 4: two fanins are inverse to each other
    //         -> replace by constant 0
    else if (in0 == ~in1)
      replaceGate(gate, const0V);
    
     
  else if (in0.gate()->getFanin0()==in1){
       if (!in0.isInv()){
            replaceGate(gate, in0);
       }
       else {
            CirGate* newgate=addAIGate("");
            connectAIG(newgate, in1.gate(), in1.isInv(), in0.gate()->getFanin1().gate(), ! in0.gate()->getFanin1().isInv());
            replaceGate(gate, CirGateV(newgate));
       }
    }

    else if (in0.gate()->getFanin0()==~in1){
       if (!in0.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            replaceGate(gate, in1);
      }
    }


    else if (in0.gate()->getFanin1()==in1){
        if (!in0.isInv()){
            replaceGate(gate, in0);
       }
       else {
              CirGate* newgate=addAIGate("");
            connectAIG(newgate, in1.gate(), in1.isInv(), in0.gate()->getFanin0().gate(), ! in0.gate()->getFanin0().isInv());
            replaceGate(gate, CirGateV(newgate));
       }
    }



    else if (in0.gate()->getFanin1()==~in1){
        if (!in0.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            replaceGate(gate, in1);
      }
    }

    

    else if (in1.gate()->getFanin0()==in0){
       if (!in1.isInv()){
            replaceGate(gate, in1);
       }
       else {
             CirGate* newgate=addAIGate("");
            connectAIG(newgate, in0.gate(), in0.isInv(), in1.gate()->getFanin1().gate(), ! in1.gate()->getFanin1().isInv());
            replaceGate(gate, CirGateV(newgate));
       }
    }
    
    else if (in1.gate()->getFanin0()==~in0){
       if (!in1.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            replaceGate(gate, in0);
      }
    }

    else if (in1.gate()->getFanin1()==in0){
       if (!in1.isInv()){
            replaceGate(gate, in1);
       }
       else {
              CirGate* newgate=addAIGate("");
            connectAIG(newgate, in0.gate(), in0.isInv(), in1.gate()->getFanin0().gate(), !in1.gate()->getFanin0().isInv());
            replaceGate(gate, CirGateV(newgate));
       }
    }


    else if (in1.gate()->getFanin1()==~in0){
          if (!in1.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            replaceGate(gate, in0);
      }
    }

    

  }

  buildAigList();
  buildTopoList();
  return sweep();
}

unsigned CirMgr::strash() {
  std::map<std::pair<CirGateV, CirGateV>, CirGate*> mp;
  
  for (unsigned i = 0, n = topoList_.size(); i < n; ++i) {
    CirGate* gate = topoList_[i];
    //if (!gate->getSelect()){continue;}
    if (gate->getType() != GATE_AIG) continue;
    CirGateV in0 = gate->getFanin0();
    CirGateV in1 = gate->getFanin1();
    assert(in0.gate() != NULL && in1.gate() != NULL);
    if (in0 > in1) std::swap(in0, in1);
    std::pair<CirGateV, CirGateV> vpair(in0, in1);
    if (mp.find(vpair) == mp.end()) {
      // pair not in map
      mp[vpair] = gate;
    } else {
    // pair in map -> replace current gate by gate in map
      CirGateV repl(mp[vpair]);
      replaceGate(gate, repl);
    

    }
  }
  buildAigList();
  buildTopoList();
  return optimize();
}
