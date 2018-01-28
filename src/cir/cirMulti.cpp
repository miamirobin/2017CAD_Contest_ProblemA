 
#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "cir/satMgr.h"
#include <time.h>


void SATMgr::buildMulti(){
    assert(!_isSingle);  
    rvset.clear();   
    rvsetVec.clear();                  
    
    cout << "Multiple Problem..." << endl;
    /*cofactor reduction
      find the rectifying vector      */
    if (ckt1_->getwtSize()==2){
         rvset.push_back(false);rvset.push_back(false);
         rvsetVec.push_back(rvset);rvset.clear();
         rvset.push_back(false);rvset.push_back(true);
         rvsetVec.push_back(rvset);rvset.clear();
         rvset.push_back(true);rvset.push_back(false);
         rvsetVec.push_back(rvset);rvset.clear();
         rvset.push_back(true);rvset.push_back(true);
         rvsetVec.push_back(rvset);rvset.clear();

    }

  


    else {
       if (!cofactorReduction()){
         cout<<"UNSAT.. "<<endl;          
     
          
       }
       else {
            cout<<"SAT.." <<endl;
       }
         
    }     

    while (true){            
          buildControlVar1() ; 
          buildMultiple();
       
          if (ckt1_->constcount>=5){NoTime =true;break;}
          sol_mul_one();

          if (NoTime ==true){ break;}
          sol_mul_two(); 
 
          buildControlVar2();
          buildMultiple();
          sol_mul_one();
          sol_mul_two();                  
          break;

   }

   if (NoTime){   
      for (int i=0;i<ckt1_->piList_.size();i++){
          if (ckt1_->piList_[i]->getSelect()){ 
                  commonVar.push_back(ckt1_->piList_[i]->getName()); 
                  BestCost+=ckt1_->piList_[i]->getWeight();
          }

      }
   }            
   
  
      for (int wt=0;wt<ckt1_->getwtSize();wt++){
          buildMultiple2(wt);
            if (!_minisatPtr->solve()){ 
			cout << "UNSAT..." << endl << endl;    
			
			_nVar = _minisatPtr->nVars();
			                      
			getItp();                            
                     
                        CirGate* poGate=ckt1_->addPoGate("t_"+ to_string(wt));   
                  	CirGate* aiGate=ckt1_->getGateByName(_patchNameVec[_patchNameVec.size()-1]); 
     
                	ckt1_->connectPo(poGate, aiGate); 
             }
      }

       
      
       if (NoTime) {ckt1_->optimize_itphalf();}
       else {   ckt1_->optimize_itp();}
      
   
       ckt1_->outputPatch();

       for (int wt=0;wt<ckt1_->getwtSize();wt++){
             CirGate* poGate=  ckt1_-> poList_[ckt1_->poList_.size()+wt-ckt1_->getwtSize()];
             
       }

        cout<<"Best Cost: "<<BestCost<<endl;          
       cout<<"Patch Size: "<<ckt1_->output.size()+ckt1_->wire.size()<<endl;      

   
}


void SATMgr::buildMultiple(){ 
        _onSetVec.clear();
       _offSetVec.clear();            
                       
   for (int i=0;i<rvsetVec.size();i++){   
      
          _onSetCir.clear();                                
          
          _onSetCir.push_back(ckt1_->duplicateSelect());      

          _onSetVec.push_back(_onSetCir);
          buildforMultiple(_onSetVec[_onSetVec.size()-1],rvsetVec[i]);          
          
          if (_onSetVec.size()>1){  
              
               addComControl(_onSetVec[_onSetVec.size()-2][0], _onSetVec[_onSetVec.size()-1][0]);
           
           
          }        
        
   }
   
   
}


void SATMgr::buildforMultiple(vector<CirMgr*>& _cirVec,vector<bool> &rvset){
  int cur_clauseNum = getNumClauses();

  
  //_cirVec[0]->addCircuitCNF(_minisatPtr);
   //_cirVec[0]->buildTopoList2();
   _cirVec[0]->addCircuitCNF(_minisatPtr);

                                                                          
  Var* _on_OR_i_Var = new Var[ (_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (size_t j=0, n =(_cirVec[0]->poneedlist.size() )/2; j < n; ++j){   
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);          // cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);       // cout<<_cirVec[0]->poneedlist[j+n]->getName();
    _on_OR_i_Var[j] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false,			       _cirVec[0]->getPoVar(k) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  
                                                                                                   
  resizeMarkVec();

  
    
   for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
        _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
   }    
      
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
   
  
   
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
    

}



void SATMgr::buildMultiple2(int wt){
      /*build onset offset    
     and then addComControl  */
   _onSetVec.clear();
   _offSetVec.clear();                      
   resetsolver() ;                 
   for (int i=0;i<rvsetVec.size();i++){   
       if (rvsetVec[i][wt]==false){
           
       
          _onSetCir.clear();                                
          //_onSetCir.push_back(ckt0_->duplicate());
          _onSetCir.push_back(ckt1_->duplicateSelect());      

          _onSetVec.push_back(_onSetCir);
          _onSetVec[_onSetVec.size()-1][0]->Cut=commonVar;
          buildONOFFset2forITP(_onSetVec[_onSetVec.size()-1],true,rvsetVec[i]);          
          
          if (_onSetVec.size()>1){  
              int cur_clauseNum=getNumClauses();
              BindPi(_onSetVec[_onSetVec.size()-2][0]->getPiList(), _onSetVec[_onSetVec.size()-1][0]->getPiList());
              resizeMarkVec();  
              for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
                      markOnSet(i);
              }


          }

        }
        else {

      
          _offSetCir.clear();
        
          _offSetCir.push_back(ckt1_->duplicateSelect());

          _offSetVec.push_back(_offSetCir);
          _offSetVec[_offSetVec.size()-1][0]->Cut=commonVar;
          buildONOFFset2forITP(_offSetVec[_offSetVec.size()-1],false,rvsetVec[i]);
          
          if (_offSetVec.size()>1){     
                  int cur_clauseNum=getNumClauses();
                  BindPi(_offSetVec[_offSetVec.size()-2][0]->getPiList(), _offSetVec[_offSetVec.size()-1][0]->getPiList());
               
  
                  resizeMarkVec();  
                  for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
                      markOffSet(i);
                  }

          }
          
         
        }          
   }
   _var2Name.clear();
   for (int i=0;i<commonVar.size();i++){ 
                 _minisatPtr->addEqCNF(_onSetVec[0][0]->getGateByName( commonVar[i])->getVar(),_offSetVec[0][0]->getGateByName( commonVar[i])->getVar(), false);
                 mapVar2Name(_onSetVec[0][0]->getGateByName( commonVar[i])->getVar(), commonVar[i]);
   }
  
   resizeMarkVec();  


}


void SATMgr::BindPi(GateList& g1, GateList& g2){     
  assert(g1.size()==g2.size());

  for (size_t i = 0, n = g1.size(); i < n; ++i){
   
    if (g1[i]->getName()!="" and g1[i]->getpoNeed()==false  ){
       
         _minisatPtr->addEqCNF(g1[i]->getVar(), g2[i]->getVar(), false);
     
    }
  }
  resizeMarkVec();   
}



void SATMgr::resetRM(){
       rm.clear();
       
       rm.push_back(ckt1_->duplicateSelect());
}

bool SATMgr::cofactorReduction(){
    resetsolver();
    rvset.clear();
    for (int i=0;i<ckt1_->getwtSize();i++){
       rvset.push_back(false);
    } 
    int qq=0;
    while (true){

   
    
       rvsetVec.push_back(rvset);
       resetRM();
       SolveRCset(rm,rvset); 
       rcset.push_back(rm);
       if (rcset.size()>1){
           BindPi(rcset[rcset.size()-2][0]->getPiList(), rcset[rcset.size()-1][0]->getPiList());
         
       }
            
       for (int j=0;j<rvset.size();j++){
          cout<<rvset[j];
       }
       cout<<" "<<qq<<"\n";
       qq++;
       rvset.clear(); 

       if (qq  >48){NoTime=true;return false;}

       if (!_minisatPtr->solve()){
          
          return false;
       }
       else {
          resetRM(); 
          
          if (!SolvenRM(rm)){ 
             return  true;
          }
          
      }
    }

}


bool SATMgr::SolvenRM(vector<CirMgr*>& _cirVec ){
   if (solver0 != NULL)	delete solver0;
   solver0 = new SatSolver();
   solver0->init();
  int cur_clauseNum = getNumClauses();
  
  //_cirVec[0]->addCircuitCNF( solver0);
  //_cirVec[0]->buildTopoList2();
  _cirVec[0]->addPoCNF( solver0);

 
                                                                            
  Var* _on_OR_i_Var = new Var[(_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = solver0->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                      
  for (size_t j=0, n = (_cirVec[0]->poneedlist.size() )/2; j < n; ++j){
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);          // cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);       
    _on_OR_i_Var[j] = solver0->newVar();                             

    // add output difference gate
    solver0->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false,    _cirVec[0]->getPoVar(k) , false);          
    // add (~Input i + Output)
    solver0->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  solver0->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
                                                                                                     
  resizeMarkVec();

  
   
   for (unsigned i=0;i<_cirVec[0]->piList_.size();i++){ 
        solver0->addClause(mkLit(_cirVec[0]->getPiVar(i), !_minisatPtr->getVal( rm[0]->getPiVar(i))));  
   }    
      
    solver0->addClause(mkLit(_on_OR_o_var, true)); // output of OR_miter must be 1  
  
   
	if(!solver0->simplify())
		cout << "ERROR in simplify" << endl;
    if (solver0->solve()){
        for (int i=0;i<ckt1_->tList_.size();i++){      
          rvset.push_back(solver0->getVal( _cirVec[0]->getWtVar(i)));
        }     
        return true;
    }
    else {
          return false;

    }
}


void SATMgr::SolveRCset(vector<CirMgr*>& _cirVec,vector<bool> &rvset){
	//SatSolver* _solver = _solMgr._minisatPtr;
  int cur_clauseNum = getNumClauses();

   _cirVec[0]->addPoCNF(_minisatPtr);

                                                                          
  Var* _on_OR_i_Var = new Var[ (_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (size_t j=0, n =(_cirVec[0]->poneedlist.size() )/2; j < n; ++j){   
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);          // cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);       // cout<<_cirVec[0]->poneedlist[j+n]->getName();
    _on_OR_i_Var[j] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false,  _cirVec[0]->getPoVar(k) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
 
                                                                                                   
  resizeMarkVec();

  
    
   for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
        _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
   }    
      
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
   
  
   
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
    
}




void SATMgr::buildONOFFset2(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset){

  int cur_clauseNum = getNumClauses();

  //_cirVec[0]->addCircuitCNF(_minisatPtr);
  //_cirVec[0]->buildTopoList2();
  _cirVec[0]->addCircuitCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[ (_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (size_t j=0, n =(_cirVec[0]->poneedlist.size() )/2; j < n; ++j){   //cout<<n;
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);          // cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);       // cout<<_cirVec[0]->poneedlist[j+n]->getName();
    _on_OR_i_Var[j] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false, _cirVec[0]->getPoVar(k) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  
                                                                                                   
  resizeMarkVec();

  if (ON_OFF){ // on set
    
      for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
  	  _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
       }    
      
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    
    // mark on set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);
    }
  }
  else{ // off set
    
    
      for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
  	  _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
       }
        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

 
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
}


    
void SATMgr::buildONOFFset2forITP(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset){

  int cur_clauseNum = getNumClauses();

  _cirVec[0]->addPoCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[ (_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (size_t j=0, n =(_cirVec[0]->poneedlist.size() )/2; j < n; ++j){  
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);          // cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);       // cout<<_cirVec[0]->poneedlist[j+n]->getName();
    _on_OR_i_Var[j] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false, _cirVec[0]->getPoVar(k) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  // bind primary input
 // for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
   //   _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  //}
                                                                                                   
  resizeMarkVec();

  if (ON_OFF){ // on set
    
      for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
  	  _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
       }    
      
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    
    // mark on set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);
    }
  }
  else{ // off set
    
    
      for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
  	  _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(i), !rvset[i]));  
       }
        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

 
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
}


    


