#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "cir/cirMgr.h"

using namespace std;


// transfer
void CirMgr::verilog( CirGate* gate){
        CirGateV in0 = gate->getFanin0();   
       CirGateV in1 = gate->getFanin1();

    

       if (in1.isNull() or in1.isFloat() or in0==in1){  

 
 
                CirGateV in00 = in0.gate()->getFanin0();
                CirGateV in11 = in0.gate()->getFanin1();
              
                if (  in11.isNull() or in11.isFloat() or in00.gate()->getNAME()=="4"  or in11.gate()->getNAME()=="4"){
                        if (in0.isInv()){
                         
                             patch.push_back( "not");
                             patch.push_back(gate ->getName());
                             patch.push_back(in0.gate()->getName());
                             Patch.push_back(patch);
                             patch.clear();
                              // if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                         
                        }
                        else {

                             patch.push_back( "buf");
                             patch.push_back(gate ->getName());
                             patch.push_back(in0.gate()->getName());
                             Patch.push_back(patch);
                             patch.clear();
                             //  if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}


                         }
                   



                 }
                else { 
                
                  if (in0.isInv()){        

                      if (in00.isInv() and in11.isInv()){   
                   	     patch.push_back("or");
                    	     patch.push_back(gate->getName());
                     	     patch.push_back(in00.gate()->getName());
                             patch.push_back(in11.gate()->getName());
                             Patch.push_back(patch);
                             patch.clear();                                        
                             in0.gate()->setToFlg();
                              //if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                 
                       }
                       else if (in00.isInv() and !in11.isInv()){
                            string nam="N"+in00.gate()->getName();
                            CirGate* Nin00=getGateByName(nam);
                           

                         patch.push_back( "nand");
                         patch.push_back(gate->getName());
                         patch.push_back(Nin00->getName());
                         patch.push_back(in11.gate()->getName());
                         Patch.push_back(patch);
                         patch.clear();
                          if (!(Nin00->isFlg())) {
                            patch.push_back( "not");
                            patch.push_back(Nin00->getName());
                            patch.push_back(in00.gate()->getName());
                            Patch.push_back(patch);
                            patch.clear();
                            Nin00->setToFlg();
                            }


                         in0.gate()->setToFlg();
                          //if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                        }
                        else if (!in00.isInv() and in11.isInv()){
                            string nam="N"+in11.gate()->getName();
                            CirGate* Nin11=getGateByName(nam);
                          

                              patch.push_back( "nand");
                              patch.push_back(gate->getName());
                              patch.push_back(in00.gate()->getName());
                              patch.push_back(Nin11->getName());
                              Patch.push_back(patch);
                              patch.clear();
                                 if (!(Nin11->isFlg())) {
                                patch.push_back( "not");
                                patch.push_back(Nin11->getName());
                                patch.push_back(in11.gate()->getName());
                                Patch.push_back(patch);
                                patch.clear();
                                Nin11->setToFlg();
                              }


                              in0.gate()->setToFlg();
                             // if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                          }
                         else if (!in00.isInv() and !in11.isInv()){
                              patch.push_back( "nand");
                              patch.push_back(gate->getName());
                              patch.push_back(in00.gate()->getName());
                              patch.push_back(in11.gate()->getName());
                              Patch.push_back(patch);
                              patch.clear();
                              in0.gate()->setToFlg();
                              //if (!in1.isNull() and !in1.isFloat() ) {in1.gate()->setToFlg();}
                     
                         }


                     }
                    else {
                   
                      if (in00.isInv() and in11.isInv()){
                        patch.push_back("nor");
                        patch.push_back(gate->getName());
                        patch.push_back(in00.gate()->getName());
                        patch.push_back(in11.gate()->getName());
                        Patch.push_back(patch);
                        patch.clear();
                        in0.gate()->setToFlg();
                       //if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                        
                       }

                      else if (in00.isInv() and !in11.isInv()){
                         string nam="N"+in00.gate()->getName();
                         CirGate* Nin00=getGateByName(nam);
                       
                  

                      patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(Nin00->getName());
                      patch.push_back(in11.gate()->getName());
                      Patch.push_back(patch);
                      patch.clear();
                       if (!(Nin00->isFlg())) {
                          patch.push_back( "not");
                          patch.push_back(Nin00->getName());
                          patch.push_back(in00.gate()->getName());
                          Patch.push_back(patch);
                          patch.clear();
                          Nin00->setToFlg();
                        }
                      in0.gate()->setToFlg();
                      // if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                       }
                      else if (!in00.isInv() and in11.isInv()){
                           string nam="N"+in11.gate()->getName();
                           CirGate* Nin11=getGateByName(nam);
                 
                  
    

                      patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(in00.gate()->getName());
                      patch.push_back(Nin11->getName());
                      Patch.push_back(patch);
                      patch.clear();
                         if (!(Nin11->isFlg())) {
                          patch.push_back( "not");
                          patch.push_back(Nin11->getName());
                          patch.push_back(in11.gate()->getName());
                          Patch.push_back(patch);
                          patch.clear();
                          Nin11->setToFlg();
                        }
                      in0.gate()->setToFlg();
                     // if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                    }
                      else if (!in00.isInv() and !in11.isInv()){


                 


                      patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(in00.gate()->getName());
                      patch.push_back(in11.gate()->getName());
                      Patch.push_back(patch);
                      patch.clear();
                      in0.gate()->setToFlg();
                     // if (!in1.isNull() and !in1.isFloat() ){in1.gate()->setToFlg();}
                     
                      }

                   }
               }
        }/*
       else if (in0==~in1){
             patch.push_back( "buf");
             patch.push_back(gate->getName());
             patch.push_back("1'b0");
             CirGate* const0=getConstGate();
             const0->setName("1'b0");
             Patch.push_back(patch);
             patch.clear();
             break;          

      }*/
       else {
           if (in0.isInv() and in1.isInv()){
               patch.push_back("nor");
                        patch.push_back(gate->getName());
                        patch.push_back(in0.gate()->getName());
                        patch.push_back(in1.gate()->getName());
                        Patch.push_back(patch);
                        patch.clear();
            
          }

          else if (in0.isInv() and !in1.isInv()){
                     string nam="N"+in0.gate()->getName();
                  CirGate* Nin0=getGateByName(nam);
                     
   

                      patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(Nin0->getName());
                      patch.push_back(in1.gate()->getName());
                      Patch.push_back(patch);
                      patch.clear();
                       if (!(Nin0->isFlg())) {
                          patch.push_back( "not");
                          patch.push_back(Nin0->getName());
                          patch.push_back(in0.gate()->getName());
                          Patch.push_back(patch);
                          patch.clear();
                          Nin0->setToFlg();
                      }

                    
          }
             
          
             
          else if (!in0.isInv() and in1.isInv()){
                       string nam="N"+in1.gate()->getName();
                     CirGate* Nin1=getGateByName(nam);
                     

 

                      patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(in0.gate()->getName());
                      patch.push_back(Nin1->getName());
                      Patch.push_back(patch);
                      patch.clear();
                       if (!(Nin1->isFlg())) {
                          patch.push_back( "not");
                          patch.push_back(Nin1->getName());
                          patch.push_back(in1.gate()->getName());
                          Patch.push_back(patch);
                          patch.clear();
                          Nin1->setToFlg();
                      }
                    
           }
           else if (!in0.isInv() and !in1.isInv()){

 


                       patch.push_back( "and");
                      patch.push_back(gate->getName());
                      patch.push_back(in0.gate()->getName());
                      patch.push_back(in1.gate()->getName());
                      Patch.push_back(patch);
                      patch.clear();
                     
                     
           }

        }

   
 



}



// transfer patch to (or ,and, nor, nand, xor, xnor, not)
void CirMgr::outputPatch(){
   buildpatchlist();         
   CirGate::incFlg();         
   int w=1;
   int g=1;
   for (int i=0;i<cutlist.size();i++){
       CirGate* gate=cutlist[i];
       gate->setName("g"+to_string(g));  //cout<<"cutlist: "<<gate->getNAME()<<endl;
       g++;
       input.push_back(gate->getNAME());
       gate->setToFlg();

   }



   for (int i=patchlist.size()-1;i>=0;--i){
                                                                        
       CirGate* gate=patchlist[i];                              
       if (gate->isFlg()) continue;
       gate->setToFlg();    
        verilog(gate);
   }

 int n=Patch.size();     // cout<<"patchlist: "<<n<<endl;
 /* 
for (int i=0;i<Patch.size();i++){   
   for (int j=0;j<Patch[i].size();j++){  
           cout<<Patch[i][j]<<" ";
   }cout<<endl;
   string name= Patch[i][1]; CirGate* gate=getGateByName(    name); 
   CirGateV in0 = gate->getFanin0();         
  CirGateV in1 = gate->getFanin1();             cout<<name<<" " ;
  if (!in0.isNull() && !in0.isFloat()){
     cout<<gate->getFanin0().isInv()<<" "<<gate->getFanin0().gate()->getName()<< " ";
  }
  if (!in1.isNull() && !in1.isFloat()){
     cout<<gate->getFanin1().isInv()<<" "<<gate->getFanin1().gate()->getName()<<" ";
  }

 
   cout<<endl;

}*/   



    
    for (int i=n-1;i>=0;--i){
       if (Patch[i][1][0]=='t' and Patch[i][1][1]=='_'){
            CirGate* gate=getGateByName(Patch[i][1]);
            gate->setName(gate->getName());
            output.push_back(gate);
       }
       else{
             CirGate* gate=getGateByName(Patch[i][1]);                        
            gate->setName("w"+to_string(w));                   
            w++;
            wire.push_back(gate);

       }
   }

   for (int i=n-1;i>=0;--i){
      if (Patch[i].size()==4){
         CirGate * in0=getGateByName (Patch[i][2]);
         if (in0->getNAME()=="5"){  
            in0->setName("w"+to_string(w));                   
            w++;
            wire.push_back(in0);
            verilog(in0);
         }
         CirGate * in1=getGateByName (Patch[i][3]);
         if (in1->getNAME()=="5"){  
            in1->setName("w"+to_string(w));                   
            w++;
            wire.push_back(in1);
            verilog(in1);
         }
      }
      else {
         CirGate * in0=getGateByName (Patch[i][2]);
          if (in0->getNAME()=="5"){  
            in0->setName("w"+to_string(w));                   
            w++;
            wire.push_back(in0);
            verilog(in0);
         }

      }

   }
   

}

void CirMgr::buildpatchlist(){
     CirGate::incFlg();
  // reset topoList and used
  patchlist.clear();
  cutlist.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  const0_->setToFlg();
  const0_->setUsed(true);
   // DFS
  for (int i=tList_.size()-1;i>=0;--i){
      CirGate* gate= poList_[poList_.size()-1-i];
      buildpatchlist_rec(gate);
  }
  
  

}


void CirMgr::buildpatchlist_rec(CirGate* gate ){            
  if (gate->isFlg()) return;
  gate->setToFlg();
  if (!(gate->getName()[0]=='i' and gate->getName()[1]=='t') and !(gate->getName()[0]=='t' and gate->getName()[1]=='_')){
      CirGate *out=addAIGate("N"+gate->getName());
      connectNot( out, gate);
      out->setName("5");
      gate->setName("5");
      cutlist.push_back(gate);         //cout<<"cut: "<<gate->getName()<<endl;

      return;
  }
  CirGateV in0 = gate->getFanin0();         
  CirGateV in1 = gate->getFanin1();       
  if (!in0.isNull() && !in0.isFloat()){
     buildpatchlist_rec(in0.gate());
  }
  if (!in1.isNull() && !in1.isFloat()){
     buildpatchlist_rec(in1.gate());
  }


  // set used and add to topoList
  gate->setUsed(true);
  CirGate *out=addAIGate("N"+gate->getName());
  connectNot( out, gate);
  out->setName("5");
  gate->setName("5");
  patchlist.push_back(gate);                 //if (gate->getName()=="itp86"){ cout<<"patchlist: "<<gate->getName()<<" "<<gate<<endl;}
  
      
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

// build topological list for target node
void CirMgr::buildt0TopoList() {
  CirGate::incFlg();
  // reset topoList and used
  t0topolist.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  const0_->setToFlg();
  const0_->setUsed(true);
   // DFS
     
  //buildt0TopoList_rec(gate); 
  for (int i=tList_.size()-1;i>=0;--i){
      CirGate* gate= poList_[poList_.size()-1-i];   
      buildt0TopoList_rec(gate);   
  }
  
  
  
}

void CirMgr::buildt0TopoList_rec(CirGate* gate) {          //cout<<gate->getName()<<endl;if(gate==NULL){cout<<gate->getName()<<end;}
  if (gate->isFlg()) return;         
  gate->setToFlg();
  CirGateV in0 = gate->getFanin0();
  CirGateV in1 = gate->getFanin1();
  if (!in0.isNull() && !in0.isFloat()){
    buildt0TopoList_rec(in0.gate());
  }
  if (!in1.isNull() && !in1.isFloat()){
    buildt0TopoList_rec(in1.gate());
  }
  // set used and add to topoList
  gate->setUsed(true);
  if (gate->getName()[0]=='i' and gate->getName()[1]=='t'){gate->setPatch();}
  t0topolist.push_back(gate);
}



void CirMgr::optimize_itp() {
  t0topolist.clear();  
  buildTopoList();   
  
  buildt0TopoList();
                    
  sweep1();
  optimize1();
  strash1();
  simulate();
  fraig1();
  //buildAigList();
  //buildTopoList();
  //buildt0TopoList(gate);
  optimize1();
}



void CirMgr::optimize_itphalf() {
  t0topolist.clear();  
  buildTopoList();   
  
  buildt0TopoList();
                    
  sweep1();
  optimize1();
  strash1();
  
 
  //buildAigList();
  //buildTopoList();
  //buildt0TopoList(gate);
  optimize1();
}





// remove unused gates
unsigned CirMgr::sweep1() {
  unsigned removeCount = 0;/*
  bool flg = true;
  while (flg) {
    flg = false;
    for (unsigned i = 0; i < gateList_.size(); ++i) {
      CirGate* gate = gateList_[i];
      assert(gate != NULL);
      if (gate->getType() != GATE_AIG) continue;
      if (gate->getFanout().size() == 0) {
        // std::cerr << "removing \"" << gate->getName() << "\"" << std::endl;
        delAIGate(i);
        --i;
        removeCount += 1;
        flg = true;
      }
    }
  }
  buildAigList();
  buildTopoList();*/
  return removeCount;
}

// do trivial optimization
unsigned CirMgr::optimize1() {
  const CirGateV const0V = CirGateV(const0_);
  
  for (unsigned i = 0, n = t0topolist.size(); i < n; ++i) {
  
    CirGate* gate =t0topolist[i];
    if (gate->getType() != GATE_AIG or !(gate->getName()[0]=='i' and gate->getName()[1]=='t') or !gate->getPatch()) continue;
    assert(gate->getType() == GATE_AIG);
    CirGateV in0 = gate->getFanin0();
    CirGateV in1 = gate->getFanin1();
    // Case 1: one of fanin is constant 1
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
    else if (in0 == in1){
       replaceGate(gate, in0);
    }
    // Case 4: two fanins are inverse to each other
    //         -> replace by constant 0
    else if (in0 == ~in1)
      replaceGate(gate, const0V);
  

  else if (in0.gate()->getFanin0()==in1){
       if (!in0.isInv()){
             if (in0.gate()->getPatch()  and in0.gate()->getName()!="" ) {replaceGate(gate, in0);}
       }
       else {
         if (in1.gate()->getPatch() and in0.gate()->getFanin1().gate()->getPatch()  and in1.gate()->getName()!="" and in0.gate()->getFanin1().gate()->getName()!="" ){
            
              string name="itpp"+  to_string(newmark);
            newmark++;
            CirGate* newgate=addAIGate(name);
            connectAIG(newgate, in1.gate(), in1.isInv(), in0.gate()->getFanin1().gate(), ! in0.gate()->getFanin1().isInv());
            replaceGate(gate, CirGateV(newgate));
         }
       }
    }

    else if (in0.gate()->getFanin0()==~in1){
       if (!in0.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            if (in1.gate()->getPatch()  and in1.gate()->getName()!="" ) {replaceGate(gate, in1);}
      }
    }


    else if (in0.gate()->getFanin1()==in1){
        if (!in0.isInv()){
            if (in0.gate()->getPatch()  and in0.gate()->getName()!="") {replaceGate(gate, in0);}
       }
       else {
          if (in1.gate()->getPatch() and in0.gate()->getFanin0().gate()->getPatch()   and in1.gate()->getName()!="" and in0.gate()->getFanin0().gate()->getName()!="" ){
              string name="itpp"+  to_string(newmark);
             newmark++;  
            CirGate* newgate=addAIGate(name);
            connectAIG(newgate, in1.gate(), in1.isInv(), in0.gate()->getFanin0().gate(), ! in0.gate()->getFanin0().isInv());
            replaceGate(gate, CirGateV(newgate));
          }
       }
    }



    else if (in0.gate()->getFanin1()==~in1){
        if (!in0.isInv()){
            replaceGate(gate, const0V);
       }
       else {
             if (in1.gate()->getPatch()  and in1.gate()->getName()!="") {replaceGate(gate, in1);}
      }
    }

    

    else if (in1.gate()->getFanin0()==in0){
       if (!in1.isInv()){
             if (in1.gate()->getPatch()  and in1.gate()->getName()!="") {replaceGate(gate, in1);}
       }
       else {
          if (in0.gate()->getPatch() and in1.gate()->getFanin1().gate()->getPatch()   and in0.gate()->getName()!="" and in1.gate()->getFanin1().gate()->getName()!=""){
            string name="itpp"+  to_string(newmark);
             
            newmark++;
            CirGate* newgate=addAIGate(name);
            connectAIG(newgate, in0.gate(), in0.isInv(), in1.gate()->getFanin1().gate(), ! in1.gate()->getFanin1().isInv());
            replaceGate(gate, CirGateV(newgate));
          }
       }
    }
    
    else if (in1.gate()->getFanin0()==~in0){
       if (!in1.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            if (in0.gate()->getPatch()  and in0.gate()->getName()!="" ){replaceGate(gate, in0);}
      }
    }

    else if (in1.gate()->getFanin1()==in0){
       if (!in1.isInv()){
            if (in1.gate()->getPatch()  and in1.gate()->getName()!="") {replaceGate(gate, in1);}
       }
       else {
         if (in0.gate()->getPatch() and in1.gate()->getFanin0().gate()->getPatch()  and in0.gate()->getName()!="" and in1.gate()->getFanin0().gate()->getName()!=""){
            string name="itpp"+  to_string(newmark);
            newmark++;
            CirGate* newgate=addAIGate(name); 
            connectAIG(newgate, in0.gate(), in0.isInv(), in1.gate()->getFanin0().gate(), !in1.gate()->getFanin0().isInv());
            replaceGate(gate, CirGateV(newgate));
         }
       }
    }


    else if (in1.gate()->getFanin1()==~in0){
          if (!in1.isInv()){
            replaceGate(gate, const0V);
       }
       else {
            if (in0.gate()->getPatch() and in0.gate()->getName()!="") {replaceGate(gate, in0);}
      }
    }

    

  }


  buildAigList();
  buildTopoList();
  buildt0TopoList();
  return sweep1();
}

unsigned CirMgr::strash1() {
  std::map<std::pair<CirGateV, CirGateV>, CirGate*> mp;
  
  for (unsigned i = 0, n = t0topolist.size(); i < n; ++i) {
    CirGate* gate = t0topolist[i];
    if (gate->getType() != GATE_AIG or !(gate->getName()[0]=='i' and gate->getName()[1]=='t') or !gate->getPatch()) continue;
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
       if (repl.gate()->getPatch() and repl.gate()->getName()!="" ){ replaceGate(gate, repl); }//replaceGate(gate, repl);
    }
  }
  buildt0TopoList();
  buildTopoList();
  buildAigList();
  return optimize1();
}







// simulate the circuit
void CirMgr::fraig1(int effort) {
  SatSolver solver;
  initSolver(&solver);
 
  for (unsigned i = 0, n = t0topolist.size(); i < n; ++i) {
    CirGate* gate = t0topolist[i];
    if (gate->getType() != GATE_AIG or !(gate->getName()[0]=='i' and gate->getName()[1]=='t') or !gate->getPatch()) continue;
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
  for (unsigned i = 0, n = t0topolist.size(); i < n; ++i) {
    
    CirGate* gate = t0topolist[i];
    if (!(gate->getName()[0]=='i' and gate->getName()[1]=='t')){continue;}
    if (gate->getEqGate().gate() != 0) {
      if (gate->getEqGate().gate()->getPatch() and gate->getEqGate().gate()->getName()!=""){
           replaceGate(gate, gate->getEqGate());
       }
    }
  }
  buildAigList();
  buildTopoList();
  buildt0TopoList();
  strash1();
}
