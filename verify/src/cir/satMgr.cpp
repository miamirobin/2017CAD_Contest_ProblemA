/****************************************************************************
  FileName     [ satMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ sam031023 ]
****************************************************************************/

#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "cir/satMgr.h"

using namespace std;


void SATMgr::reset(){
	if (_minisatPtr != NULL)	delete _minisatPtr;
	_minisatPtr = new SatSolver();
	_minisatPtr->init();
	if (ckt0_ != NULL) delete ckt0_;
        if (ckt1_ != NULL) delete ckt1_;
        ckt0_ = ckt1_ = NULL;
}


void SATMgr::resetsolver(){
	if (_minisatPtr != NULL)	delete _minisatPtr;
	_minisatPtr = new SatSolver();
	_minisatPtr->init();
        _var2Name.clear();
        _isClauseOn.clear(); 
        _varGroup.clear();
	
}


void SATMgr::readVerilog(const string& filename0, const string& filename1, const string& filename2, const string& filename3) {
  reset();
  ckt0_ = new CirMgr;
  ckt1_ = new CirMgr;         
  if (!ckt0_->readVerilog(filename0) || !ckt1_->readVerilog(filename1)){
    cout << "ERROR:: parsing error!" << endl;
    
    return;
  }                       
  
  if (!ckt1_->readparse(filename3) ){
    cout << "ERROR:: parsing error!" << endl;
    
    return;
  }    

 if ( !ckt1_->readout(filename2)){

 }

  int i= ckt1_->poList_.size();
  int onum=0;
  int inum=0;
  while (true){
   string name= ckt1_->poList_[i-1]->getName();
   if (name[0]=='.'){onum++;}
  else {break;}
    i--;
 }
  int j= ckt1_->piList_.size();
  while (true){
    string name = ckt1_->piList_[j-1]->getName();
     if (name[0]=='.'){inum++;}
      else {break;}
    j--;

  }
  for (int i=0;i<inum;i++){
      CirGate * pigate=  ckt1_->piList_[ ckt1_->piList_.size()-1-i];
      string name=pigate->getNAME();                  
      CirGate * gate= ckt1_->getGateByName( name );
       ckt1_->replaceGate( pigate, CirGateV(gate) );  
 
   }
  for (int i=0;i<onum;i++){
      CirGate * pogate=  ckt1_->poList_[ ckt1_->poList_.size()-1-i];
      string name=pogate->getName();                 
      CirGate * gate= ckt1_->getGateByName( name );  
       string names=gate->getNAME();                         
         CirGate *te= ckt1_->getGateByName( names );        
       ckt1_->replaceGate( te, CirGateV(pogate) );  
 
   }
   ckt1_->buildAigList();
   ckt1_->buildTopoList();
  
	if (ckt1_->getwtSize()==1)  _isSingle = true;
	else	_isSingle = false;      
        
       
  
   
    

}




void SATMgr::build(){
	if (_isSingle){
		cout << "Single Problem..." << endl;    
		//buildSingle();
		
                //addCommonVar(_onSetCir[1]->getAigList(), _offSetCir[1]->getAigList());
                //addCommonVar(_onSetCir[1]->getPiList(), _offSetCir[1]->getPiList());
                buildforcheckEq();                     
			if (!checkEq(_checkCir)){
				cout << "Answer Verified" << endl;
			}
			else{
                          cout << "GG" << endl;
	                }
        }
        else {  
                 
                 cout << "Multiple Problem..." << endl;
                    buildforcheckEq();                     
			if (!checkEq(_checkCir)){
				cout << "Answer Verified" << endl;
			}
			else{
                          cout << "GG" << endl;
			}
        }
}


void SATMgr::writepatch(const string& filenamef,const string& filename,const string& filename1){
        ckt1_->buildt0TopoList();
	ckt1_->writePatch(filenamef,filename,filename1);
}


void SATMgr::addCommonVar(GateList& g1, GateList& g2){     
  assert(g1.size()==g2.size());

  for (size_t i = 0, n = g1.size(); i < n; ++i){
   
    if (g1[i]->getName()!="" and g1[i]->getpoNeed()==false  ){
       
      _minisatPtr->addEqCNF(g1[i]->getVar(), g2[i]->getVar(), false);
      mapVar2Name(g1[i]->getVar(), g1[i]->getName());
    }
  }
  resizeMarkVec();   
}

void SATMgr::optimize_patch(vector<string>& _patchName,int i ){

  
    	CirGate* poGate=ckt1_->addPoGate("t_"+ to_string(i));   
    	CirGate* aiGate=ckt1_->getGateByName(_patchName[_patchName.size()-1]); 
    
    	ckt1_->connectPo(poGate, aiGate);                
        //if (ckt1_->poneedlist.size()<9) {
           ckt1_->optimize_full();  
        //}
        ckt1_->reportAllPo();
        if (i<=0){
        ckt1_->replaceGate(ckt1_->getWtGate(i), CirGateV(poGate) );           
        }
       

}

bool SATMgr::checkEq(vector<CirMgr*>& _cirVec){
   resetsolver();
   
  int cur_clauseNum = getNumClauses();

  _cirVec[0]->addCircuitCNF(_minisatPtr);
  _cirVec[1]->addCircuitCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                         
  for (size_t j=0, n =ckt1_->poneedlist.size(); j < n; ++j){
   //for (size_t i=84, n =85; i < n; ++i){
    int i= ckt1_->getPoIdx(ckt1_->poneedlist[j]);  // if (i!=98 and i!= 130)    {continue;}
    _on_OR_i_Var[i] = _minisatPtr->newVar();                             
      //cout<<_cirVec[0]->poList_[i]->getName()<<" "<<_cirVec[1]->poList_[i]->getName()<<endl;
    // add output difference gate        
    _minisatPtr->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,  _cirVec[1]->getPoVar(i) , false);   //cout<<_cirVec[0]->poList_[i]->getName()<<" "<<         _cirVec[1]->poList_[i]->getName()   <<endl;                           
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false); //cout<<_cirVec[0]->piList_[i]->getName()<<" "<<      _cirVec[1]->piList_[i]->getName()   <<endl;                           
  }/*
    for (size_t j=0, n =_cirVec[1]->tList_.size(); j < n; ++j){   

     CirGate * pogate=  _cirVec[1]->poList_[ ckt1_->poList_.size()-1-j];                        cout<<pogate->getName()<<" ";
      CirGate * togate= _cirVec[1]->tList_[ ckt1_->tList_.size()-1-j];    cout<<togate->getName()<<endl;
       _minisatPtr->addEqCNF(togate->getVar(), pogate->getVar(), false);
  }*/

         
     _minisatPtr->addClause(mkLit(_on_OR_o_var, false));

 /*
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;*/

   return _minisatPtr->solve();
 
}

void SATMgr::buildforcheckEq(){
  assert(_isSingle);
  _checkCir.push_back(ckt0_->duplicate());
  _checkCir.push_back(ckt1_->duplicate());
  
  
}




void SATMgr::buildSingle(){
  assert(_isSingle);
  _onSetCir.push_back(ckt0_->duplicate());
  _onSetCir.push_back(ckt1_->duplicate());
  buildONOFFset(_onSetCir, true, 0); // on set
   _offSetCir.push_back(ckt0_->duplicate());
  _offSetCir.push_back(ckt1_->duplicate());
  buildONOFFset(_offSetCir, false, 0); // off set
}


void SATMgr::buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit){
	//SatSolver* _solver = _solMgr._minisatPtr;
  int cur_clauseNum = getNumClauses();  

  _cirVec[0]->poNeedNumlist =  _cirVec[1]->poNeedNumlist; 
  _cirVec[0]->addPoCNF( _minisatPtr);
  _cirVec[1]->addPoCNF( _minisatPtr);
 
/*
  _cirVec[0]->addCircuitCNF(_minisatPtr);
  _cirVec[1]->addCircuitCNF(_minisatPtr);*/
                                                                          
  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                         
  for (size_t j=0, n =ckt1_->poneedlist.size(); j < n; ++j){
     int i= ckt1_->getPoIdx(ckt1_->poneedlist[j]);       
    _on_OR_i_Var[i] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,			       _cirVec[1]->getPoVar(i) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                   
                                                                                                 
  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }
                                                                                               
  resizeMarkVec(); 

  if (ON_OFF){ // on set
    _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(bit), true));        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set     
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);  

    }
  }
  else{ // off set
    _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(bit), false));
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

 
}


void SATMgr::resetRM(){
       rm.clear();
       rm.push_back(ckt0_->duplicate());
       rm.push_back(ckt1_->duplicate());
       
       rm[0]->poNeedNumlist=rm[1]->poNeedNumlist;
       rm[0]->selectPi();
       for (int i=0;i< rm[0]->piNeedList.size();i++){
               rm[1]->piNeedList.push_back(rm[1]->piList_[rm[0]->getPiIdx(rm[0]->piNeedList[i])]);
       }
       //cout<< rm[0]->getPiList().size()<<" "<<rm[0]->getSelPiList().size()<<endl;
}

bool SATMgr::cofactorReduction(){
    resetsolver();
    assert(!_isSingle);   
    for (int i=0;i<ckt1_->getwtSize();i++){
          rvset.push_back(false);
    }  
    int p=0;
    while (true){
       ++p;
       rvsetVec.push_back(rvset);
       resetRM();
       SolveRCset(rm,rvset); 
       rcset.push_back(rm);
       if (rcset.size()>1){    
          addCommonVar(rcset[rcset.size()-2][0]->getSelPiList(), rcset[rcset.size()-1][0]->getSelPiList());
          //addCommonVar(rcset[rcset.size()-2][0]->getAigList(), rcset[rcset.size()-1][0]->getAigList());
       }
     
         for (int j=0;j<rvset.size();j++){
                             cout<<rvset[j];
                         }
                         cout<<" "<<p<<"\n";


       rvset.clear(); 
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

  _cirVec[0]->poNeedNumlist =  _cirVec[1]->poNeedNumlist; 
  _cirVec[0]->addPoCNF(solver0);
  _cirVec[1]->addPoCNF(solver0);


  /*                                       
  _cirVec[0]->addCircuitCNF(solver0);
  _cirVec[1]->addCircuitCNF(solver0);*/
                                                                            
  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];                        
  Var  _on_OR_o_var = solver0->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                      
  for (size_t j=0, n =ckt1_->poneedlist.size(); j < n; ++j){
     int i= ckt1_->getPoIdx(ckt1_->poneedlist[j]);       
    _on_OR_i_Var[i] = solver0->newVar();                             

    // add output difference gate
    solver0->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,
																						_cirVec[1]->getPoVar(i) , false);                                                    
    // add (~Input i + Output)
    solver0->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }  
  solver0->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
 /*                                                                                                
  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
     solver0->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }*/

  // bind primary input
  for (size_t i=0, n = _cirVec[0]-> getSelPiNum(); i < n; ++i){
     solver0->addEqCNF(_cirVec[0]->getSelPiVar(i), _cirVec[1]->getSelPiVar(i), false);
  }
                                                                                                   
  resizeMarkVec();

  
   
   for (unsigned i=0;i<ckt1_-> getPiNum();i++){ 
        solver0->addClause(mkLit(_cirVec[1]->getPiVar(i), !_minisatPtr->getVal( rm[1]->getPiVar(i))));  
   }    
      
    solver0->addClause(mkLit(_on_OR_o_var, true)); // output of OR_miter must be 1  
  
   /*
	if(!solver0->simplify())
		cout << "ERROR in simplify" << endl;*/
    if (solver0->solve()){
        for (int i=0;i<ckt1_->tList_.size();i++){      
          rvset.push_back(solver0->getVal( _cirVec[1]->getWtVar(i)));
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
/*
  _cirVec[0]->addCircuitCNF(_minisatPtr);
  _cirVec[1]->addCircuitCNF(_minisatPtr);*/
  _cirVec[0]->poNeedNumlist =  _cirVec[1]->poNeedNumlist; 
  _cirVec[0]->addPoCNF(_minisatPtr);
  _cirVec[1]->addPoCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                         
  for (size_t j=0, n =ckt1_->poneedlist.size(); j < n; ++j){
     int i= ckt1_->getPoIdx(ckt1_->poneedlist[j]);       
    _on_OR_i_Var[i] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,
																						_cirVec[1]->getPoVar(i) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  /*                                                                                                
  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
     _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }*/

  // bind primary input
  for (size_t i=0, n = _cirVec[0]-> getSelPiNum(); i < n; ++i){
     _minisatPtr->addEqCNF(_cirVec[0]->getSelPiVar(i), _cirVec[1]->getSelPiVar(i), false);
  }
                                                                                                   
  resizeMarkVec();

  
    
   for (unsigned i=0;i<ckt1_->tList_.size();i++){ 
        _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(i), !rvset[i]));  
   }    
      
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
   
  
   /*
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
    */
}


void SATMgr::buildONOFFset2(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset){
	//SatSolver* _solver = _solMgr._minisatPtr;
  int cur_clauseNum = getNumClauses();

  _cirVec[0]->addCircuitCNF(_minisatPtr);
  _cirVec[1]->addCircuitCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                         
  for (size_t j=0, n =ckt1_->poneedlist.size(); j < n; ++j){
     int i= ckt1_->getPoIdx(ckt1_->poneedlist[j]);       
    _on_OR_i_Var[i] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,
																						_cirVec[1]->getPoVar(i) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                  
                                                                                                 
  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }
                                                                                                   
  resizeMarkVec();

  if (ON_OFF){ // on set
    if(t_num==0){
      for (unsigned i=0;i<ckt1_->tList_.size()-1;i++){ cout<<t_num<<"\n";
  	  _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(i), rvset[i]));  
       }
    }
    if (t_num==1){
       for (unsigned i=0;i<ckt1_->tList_.size();i++){cout<<t_num<<"\n";
  	  _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(ckt1_->tList_.size()-1-i), rvset[i]));  
       }
    }        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);
    }
  }
  else{ // off set
    
     if(t_num==0){
      for (unsigned i=0;i<ckt1_->tList_.size()-1;i++){ cout<<t_num<<"\n";
  	  _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(i), rvset[i]));  
       }
    }
    if (t_num==1){
       for (unsigned i=0;i<ckt1_->tList_.size();i++){ cout<<t_num<<"\n";
  	  _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(ckt1_->tList_.size()-1-i), rvset[i]));  
       }
    }          
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

 /*
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
           */
}


void SATMgr::mapVar2Name(const Var& var, const string& name){
	assert(_var2Name.find(var)==_var2Name.end());
	_var2Name[var] = name;
}
/*
void SATMgr::markOnSet(const ClauseId cid){
	unsigned cSize = getNumClauses();
	assert(cid < (int)cSize);
	if (_isClauseOn.size() < cSize){
		_isClauseOn.resize(cSize, false);
	}
	_isClauseOn[cid] = true;
}

void SATMgr::markOffSet(const ClauseId cid){
	unsigned cSize = getNumClauses();
	assert(cid < (int)cSize);
	if (_isClauseOn.size() < cSize){
		_isClauseOn.resize(cSize, false);
	}
	_isClauseOn[cid] = false;
}
*/
void SATMgr::markOnSet(const int clauseId){
	unsigned cSize = getNumClauses();
	assert(clauseId < (int)cSize);
	if (_isClauseOn.size() < cSize){
		_isClauseOn.resize(cSize, false);
	}
	_isClauseOn[clauseId] = true;
}

void SATMgr::markOffSet(const int clauseId){
	unsigned cSize = getNumClauses();
	assert(clauseId < (int)cSize);
	if (_isClauseOn.size() < cSize){
		_isClauseOn.resize(cSize, false);
	}
	_isClauseOn[clauseId] = false;
}

void SATMgr::resizeMarkVec(){
	if (_isClauseOn.size() < getNumClauses())
		_isClauseOn.resize(getNumClauses(), false);
}

/*
void SATMgr::getItp(){
    assert(_minisatPtr);
    assert(_minisatPtr->solver_->proof);

    vector<Clause> unsatCore;
    unsatCore.clear();

    // save proof log
    string proofname=  "CAD2017.itp";
    _minisatPtr->solver_->proof->save(proofname.c_str());

    string _itpName = buildItp(proofname);

		_patchNameVec.push_back(_itpName);
		unlink(proofname.c_str());
}



void SATMgr::retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const {
    unsigned int tmp, cid, idx, tmp_cid;

   // Clear all
   vector<unsigned int> clausePos;
   clausePos.clear();
   unsatCore.clear();

   // Generate clausePos
   assert(!rdr.null());
   rdr.seek(0);
   for (unsigned int pos = 0; (tmp = rdr.get64()) != RDR_EOF ;pos = rdr.Current_Pos()) {
      cid = clausePos.size();
      clausePos.push_back(pos);
      if ((tmp & 1) == 0) {  // root clause
         while ((tmp = rdr.get64()) != 0) {}
      } else {              // learnt clause
         idx = 0;
         while ((tmp = rdr.get64()) != 0) { idx = 1; }
         if (idx == 0) clausePos.pop_back(); // Clause Deleted
      }
   }

   // Generate unsatCore
   priority_queue<unsigned int> clause_queue;
   vector<bool> in_queue;
   in_queue.resize(clausePos.size());
   for(unsigned int i = 0; i < in_queue.size(); ++i) in_queue[i] = false;
   in_queue[in_queue.size() - 1] = true;
   clause_queue.push(clausePos.size() - 1); //Push leaf (empty) clause
   while (clause_queue.size() != 0) {
      cid = clause_queue.top();
      clause_queue.pop();

      rdr.seek(clausePos[cid]);

      tmp = rdr.get64();
      if ((tmp & 1) == 0) {
         //root clause
         vec<Lit> lits;
         idx = tmp >> 1;
         lits.push(toLit(idx));
         while (_varGroup[idx >> 1] != COMMON){
            tmp = rdr.get64();
            if (tmp == 0) break;
            idx += tmp;
            lits.push(toLit(idx));
         }
         unsatCore.push_back(Clause(false, lits));
      } else {
         //derived clause
         tmp_cid = cid - (tmp >> 1);
         if (!in_queue[tmp_cid]) {
            in_queue[tmp_cid] = true;
            clause_queue.push(tmp_cid);
         }
         while (1) {
            tmp = rdr.get64();
            if (tmp == 0) break;
            tmp_cid = cid - rdr.get64();
            if (!in_queue[tmp_cid]) {
               in_queue[tmp_cid] = true;
               clause_queue.push(tmp_cid);
            }
         }
      }
   }
}

void SATMgr::retrieveProof(Reader& rdr, vector<unsigned int>& clausePos, vector<ClauseId>& usedClause) {
   unsigned int tmp, cid, idx, tmp_cid, root_cid;

   // Clear all
   clausePos.clear();
   usedClause.clear();
   _varGroup.clear();
   //_varGroup.resize(_minisatPtr->nVars(), NONE);
   _varGroup.resize(_nVar, NONE);
   _isClaOnDup.clear();//_isClaOnDup.reserve(_isClauseOn.size());
   assert((int)_isClauseOn.size() == getNumClauses());assert(_isClauseOn.size()!=0);

   // Generate clausePos && varGroup
   assert(!rdr.null());
   rdr.seek(0);
   root_cid = 0;
   for (unsigned int pos = 0; (tmp = rdr.get64()) != RDR_EOF ;pos = rdr.Current_Pos()) {
      cid = clausePos.size();
      clausePos.push_back(pos);
      if ((tmp & 1) == 0) {
         //Root Clause
         bool _tmmmmp = _isClauseOn[root_cid];
         _isClaOnDup.push_back(_tmmmmp);
         idx = tmp >> 1;
         if (_isClauseOn[root_cid]) {
            if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_ON;
            else if (_varGroup[idx >> 1] == LOCAL_OFF) _varGroup[idx >> 1] = COMMON;
         }
         else {
            if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_OFF;
            else if (_varGroup[idx >> 1] == LOCAL_ON) _varGroup[idx >> 1] = COMMON;
         }
         while(1) {
            tmp = rdr.get64();
            if (tmp == 0) break;
            idx += tmp;
            if (_isClauseOn[root_cid]) {
               if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_ON;
               else if (_varGroup[idx >> 1] == LOCAL_OFF) _varGroup[idx >> 1] = COMMON;
            }
            else {
               if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_OFF;
               else if (_varGroup[idx >> 1] == LOCAL_ON) _varGroup[idx >> 1] = COMMON;
            }
         }
         ++root_cid;
      }
      else {
         _isClaOnDup.push_back(false);
         idx = 0;
         while (1) {
            tmp = rdr.get64();
            if (tmp == 0) break;
            idx = 1;
            tmp = rdr.get64();
         }
         if (idx == 0) {
            clausePos.pop_back(); // Clause Deleted
            _isClaOnDup.pop_back(); // Clause Deleted
         }
      }
   }

   // Generate usedClause
   priority_queue<unsigned int> clause_queue;
   vector<bool> in_queue;
   in_queue.resize(clausePos.size());
   for(unsigned int i = 0; i < in_queue.size(); ++i ) in_queue[i] = false;
   in_queue[in_queue.size() - 1] = true;
   clause_queue.push(clausePos.size() - 1); //Push root empty clause
   while (clause_queue.size() != 0) {
      cid = clause_queue.top();
      clause_queue.pop();

      rdr.seek(clausePos[cid]);

      tmp = rdr.get64();
      if ((tmp & 1) == 0) continue; //root clause

      // else, derived clause
      tmp_cid = cid - (tmp >> 1);
      if(!in_queue[ tmp_cid ]) {
         in_queue[tmp_cid] = true;
         clause_queue.push(tmp_cid);
      }
      while (1) {
         tmp = rdr.get64();
         if (tmp == 0) break;
         tmp_cid = cid - rdr.get64();
         if (!in_queue[tmp_cid]) {
            in_queue[tmp_cid] = true;
            clause_queue.push(tmp_cid);
         }
      }
   }
   for (unsigned int i = 0; i < in_queue.size(); ++i) {
      if (in_queue[i]) {
         usedClause.push_back(i);
      }
   }
}

string SATMgr::buildItp(const string& proofName) {
   Reader rdr;
   // records
   map<ClauseId, string> claItpLookup;
   vector<unsigned int> clausePos;
   vector<ClauseId> usedClause;
   // ntk
   //uint32_t netSize = _ntk->getNetSize();
   // temperate variables
   string nId, nId1, nId2,nIdd;
   
   int i, cid, tmp, idx, tmp_cid;
   // const 1 & const 0
   string CONST0, CONST1;
   CONST0 = "1'b0";
   CONST1 = "1'b1";

    string _itpMark = "itp";
   int _itpNewMark = 0;

   rdr.open( proofName.c_str() );
   retrieveProof( rdr, clausePos, usedClause );    

   for (i = 0; i < (int)usedClause.size() ; i++) {

     
      cid= usedClause[i];
      rdr.seek(clausePos[cid]);  
      tmp = rdr.get64();                
      if ((tmp & 1) == 0) {
         // Root Clause
         if (_isClaOnDup[cid]) {
            idx = tmp >> 1;
            while (_varGroup[idx >> 1] != COMMON ) {
               tmp = rdr.get64();
               if (tmp == 0) break;
               idx += tmp;
            }
            if (_varGroup[idx >> 1] == COMMON) {
               assert(_var2Name.find(idx >> 1) != _var2Name.end());
               nId = (_var2Name.find(idx >> 1))->second;                 
               nId1 = (_var2Name.find(idx >> 1))->second;                 
               //if ((idx & 1) == 1) nId1 = ~nId1; // need modify
               //if ((idx & 1) == 1) nId = ~nId;// need modify
               if ((idx & 1) == 1) {
                       string _tmpName = _itpMark + to_string(_itpNewMark);
                       CirGate* tmpGate = ckt1_->addAIGate(_tmpName);
                       ckt1_->connectNot(tmpGate, ckt1_->getGateByName(nId1));
                       nId1 = _tmpName;              
                       _itpNewMark++;
               }
               if ((idx & 1) == 1){
                       string _tmpName = _itpMark + to_string(_itpNewMark);
                       CirGate* tmpGate = ckt1_->addAIGate(_tmpName);
                       ckt1_->connectNot(tmpGate, ckt1_->getGateByName(nId));
                       nId = _tmpName;              
                       _itpNewMark++;
               }
               while (1) {
                  tmp = rdr.get64();
                  if (tmp == 0) break;
                  idx += tmp;
                  if (_varGroup[idx >> 1] == COMMON) {
                     assert(_var2Name.find(idx >> 1) != _var2Name.end());
                     nId2 = (_var2Name.find(idx >> 1))->second;            
                     //if ((idx & 1) == 1) nId2 = ~nId2;
                     if ((idx & 1) == 1){
                       string _tmpName = _itpMark + to_string(_itpNewMark);
                       CirGate* tmpGate = ckt1_->addAIGate(_tmpName);
                       ckt1_->connectNot(tmpGate, ckt1_->getGateByName(nId2));
                       nId2 = _tmpName;              
                       _itpNewMark++;
                      }
                     // or
                    
                     nId = _itpMark + to_string(_itpNewMark);
                     nIdd = _itpMark + to_string(_itpNewMark+1);
                     CirGate* outGate = ckt1_->addAIGate(nId);
                     CirGate* tmpGate = ckt1_->addAIGate(nIdd);
                     ckt1_->connectNot(outGate, tmpGate);

                     ckt1_->connectAIG(tmpGate, ckt1_->getGateByName(nId1), true, ckt1_->getGateByName(nId2), true);
                     _itpNewMark+=2;
                     nId1 = nId;  

                  }
   
                     
               }
            }
            else {
               nId = CONST0;
               
            }

            claItpLookup[cid] = nId;
         }
         else {
            claItpLookup[cid] = CONST1;
         }
      }
      else {
         // Derived Clause
         tmp_cid = cid - (tmp >> 1);
         assert(claItpLookup.find(tmp_cid) != claItpLookup.end());
         nId = (claItpLookup.find(tmp_cid))->second;             
         nId1 = (claItpLookup.find(tmp_cid))->second;        
         while (1) {
            idx = rdr.get64();
            if (idx == 0) break;
            idx--;
            // Var is idx
            tmp_cid = cid - rdr.get64();
            assert(claItpLookup.find(tmp_cid) != claItpLookup.end());
            nId2 = (claItpLookup.find(tmp_cid))->second;                      
            if (nId1 != nId2) {
               if (_varGroup[idx] == LOCAL_ON) { // Local to A. Build OR Gate.
                  if (nId1 == CONST1 || nId2 == CONST1) {
                     nId = CONST1;
                     nId1 = nId;
                  }
                  else if(nId1 == CONST0) {
                     nId = nId2;
                     nId1 = nId;
                  }
                  else if(nId2 == CONST0) {
                     nId = nId1;
                     nId1 = nId;
                  }
                  else {
                     // or
                    
                     nId = _itpMark + to_string(_itpNewMark);
                     nIdd = _itpMark + to_string(_itpNewMark+1);
                     CirGate* outGate = ckt1_->addAIGate(nId);
                     CirGate* tmpGate = ckt1_->addAIGate(nIdd);
                     ckt1_->connectNot(outGate, tmpGate);
                     ckt1_->connectAIG(tmpGate, ckt1_->getGateByName(nId1), true, ckt1_->getGateByName(nId2), true);
                     _itpNewMark+=2;

                     nId1 = nId;
                  }
               }
               else { // Build AND Gate.
                  if (nId1 == CONST0 || nId2 == CONST0) {
                     nId = CONST0;
                     nId1 = nId;
                  }
                  else if(nId1 == CONST1) {
                     nId = nId2;
                     nId1 = nId;
                  }
                  else if(nId2 == CONST1) {
                     nId = nId1;
                     nId1 = nId;
                  }
                  else {
                     // and
                     
                     nId = _itpMark + to_string(_itpNewMark);
                     CirGate* outGate = ckt1_->addAIGate(nId);
                     ckt1_->connectAIG(outGate, ckt1_->getGateByName(nId1), false, ckt1_->getGateByName(nId2), false);
                     _itpNewMark++;

                     nId1 = nId;
                  }
               }
            }

    

         }
         claItpLookup[cid] = nId;
      }
   }

   cid = usedClause[usedClause.size() - 1];
   nId = claItpLookup[cid];

   //for (int i=0;i<claItpLookup.size();i++){cout<<claItpLookup[i]<<endl;}
   //_ptrMinisat->resizeNtkData(_ntk->getNetSize() - netSize); // resize Solver data to ntk size

   return nId;
}
*/
