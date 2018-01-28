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
#include <algorithm>
#include <time.h>

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


void SATMgr::readVerilog(const string& filename0, const string& filename1,  const string& filename2) {
    reset();
    ckt0_ = new CirMgr();
    ckt1_ = new CirMgr();        
    if (!ckt0_->readVerilog(filename0) || !ckt1_->readVerilog(filename1)){
       cout << "ERROR:: parsing error!" << endl;
    
       return;
    }
    ckt1_->readWeight(filename2);
    ckt1_->optimize_half();
    ckt0_->optimize_half();
  
    ckt1_->selectPo();   
    ckt0_->poNeedNumlist=ckt1_->poNeedNumlist;
    ckt0_->selectPi();
    for (int i=0;i< ckt0_->piNeedList.size();i++){
              ckt1_->piNeedList.push_back(ckt1_->piList_[ckt0_->getPiIdx(ckt0_->piNeedList[i])]);
    }
    


    

    
    ckt1_->selectGate();                        //cout<<ckt1_->getGateByName(name)->getSelect()<<endl;
    if (ckt1_->bestWeight>=100){ckt1_->selectOther();}
    ckt1_->removeGate();                       //cout<<ckt1_->getGateByName(name)->getSelect()<<endl;
    
    ckt0_->duplicateGold(ckt1_);
  
              
    ckt1_->optimize_full();
    
  
    ckt1_->selectPo();              
    ckt1_->selectPi();
    ckt1_->selectGate();                          //    cout<<ckt1_->getGateByName(name)->getSelect()<<endl;
    if (ckt1_->bestWeight>=100){ckt1_->selectOther();}    
    ckt1_->removeGate();                              //  cout<<ckt1_->getGateByName(name)->getSelect()<<endl;
    //ckt1_->reportAllPo();

    for (int i=0;i<ckt1_->getGateNum();i++){  
          if ( !ckt1_->gateList_[i]->getSelect() or ckt1_->gateList_[i]->getpoNeed() or ckt1_->gateList_[i]->getName()=="" 
                                            or ckt1_->gateList_[i]->getWeight()==-1 or ckt1_->gateList_[i]->getOther())      
         {continue;}
 
         _weightMap[ckt1_->gateList_[i]->getName()] = (ckt1_->gateList_[i]->getWeight());        // cout<<ckt1_->gateList_[i]->getName()<<endl;
         WandL.push_back(ckt1_->gateList_[i]->getWeight());
         WandL.push_back(ckt1_->gateList_[i]->getLevel());
         assert(WandL.size()==2);
         _weightOrder.push_back(make_pair( WandL , ckt1_->gateList_[i]->getName() ));
         WandL.clear();
    }

    if (ckt1_->getwtSize()==1)  _isSingle = true;
    else	_isSingle = false;      
      
    
}

void SATMgr::buildControlVar1(){
       resetsolver();
       sort(_weightOrder.begin(), _weightOrder.end(),myComparison1);
      
       _OrderedVar = new Var[_weightOrder.size()];
       for (unsigned i=0, n=_weightOrder.size(); i<n; ++i){              
            _OrderedVar[i] = _minisatPtr->newVar();

            assert(_gate2ConVar.find(_weightOrder[i].second)==_gate2ConVar.end());
            _gate2ConVar[_weightOrder[i].second] = _OrderedVar[i];
       }

}



void SATMgr::buildControlVar2(){
       resetsolver();
      
       sort(_weightOrder.begin(), _weightOrder.end(),myComparison2);
       _OrderedVar = new Var[_weightOrder.size()];
       for (unsigned i=0, n=_weightOrder.size(); i<n; ++i){
          _OrderedVar[i] = _minisatPtr->newVar();

          assert(_gate2ConVar.find(_weightOrder[i].second)==_gate2ConVar.end());
          _gate2ConVar[_weightOrder[i].second] = _OrderedVar[i];
       }

}







void SATMgr::build(){
	if (_isSingle){  
		cout << "Single Problem..." << endl;
                while (true){
                   buildControlVar1();    
		   buildSingle();
                   sol_sin_one();
                   if (commonVar.size()>0){break;}    
                   sol_sin_minus();
                   sol_sin_zero();
                 
                  
                  
                  if (   (double)clock() / CLOCKS_PER_SEC  >1500) {NoTime=true; break;}

                   buildControlVar1();
		   buildSingleSS();
	           sol_sin_minus();
                   sol_sin_zero();

                   
	           buildControlVar2(); 
                   buildSingle();
                   sol_sin_minus();
                   sol_sin_zero();


                   break;

		}
               
               
                resetsolver();
                buildSingle2();  
                for (int i=0;i<commonVar.size();i++){   
                      ckt1_->getGateByName(commonVar[i])->setPatch();
                      _minisatPtr->addEqCNF(_onSetCir[0]->getGateByName( commonVar[i])->getVar(),_offSetCir[0]->getGateByName( commonVar[i])->getVar(), false);
                      mapVar2Name(_onSetCir[0]->getGateByName( commonVar[i])->getVar(), commonVar[i]);
                 }
  
                 resizeMarkVec();  
                
        
             if (!_minisatPtr->solve()){ 
			cout << "UNSAT..." << endl << endl;    
			
			_nVar = _minisatPtr->nVars();
			                      
			getItp();                          
                        optimize_patch(_patchNameVec,0);        
                
              }
              else{                           
		  cout << "SAT" << endl;
              }

        }
        else {  
                
               buildMulti();    
               
        }
}


void SATMgr::writepatch(const string& filenamef,const string& filename,const string& filename1){
       
	ckt1_->writePatch(filenamef,filename,filename1);
}


void SATMgr::addCommonVar(GateList& g1, GateList& g2){     
  assert(g1.size()==g2.size());

  for (size_t i = 0, n = g1.size(); i < n; ++i){
   
    if (g1[i]->getName()!=""   ){
       
      _minisatPtr->addEqCNF(g1[i]->getVar(), g2[i]->getVar(), false);
      mapVar2Name(g1[i]->getVar(), g1[i]->getName());
    }
  }
  resizeMarkVec();   
}


void SATMgr::addComControl(CirMgr* c1, CirMgr* c2){
	 
	assert (c1->getGateNum() == c2->getGateNum());    
	for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
		Var v1,v2;
		CirGate* _g1;
		CirGate* _g2;
		
                _g1 = c1->getGateByName(_weightOrder[i].second); 
                _g2 = c2->getGateByName(_weightOrder[i].second);    
                if (_g1 == NULL || _g2 == NULL){
                     cout  <<"NULL gate "<< _weightOrder[i].second << endl;
                }
                assert(_g1!=NULL); assert(_g2!=NULL);
		v1 = _g1->getVar(); //cout << v1 << endl;
		v2 = _g2->getVar(); //cout << v2 << endl;
		
                assert(v1!=0); assert(v2!=0); assert(v1!=v2); assert(_OrderedVar[i]!=0);
      
		_minisatPtr->addClause(mkLit(_OrderedVar[i], true), mkLit(v1, false), mkLit(v2, true));
		_minisatPtr->addClause(mkLit(_OrderedVar[i], true), mkLit(v1, true), mkLit(v2, false));

	}
        resizeMarkVec();
}


void SATMgr::optimize_patch(vector<string>& _patchName,int i ){

  
    	CirGate* poGate=ckt1_->addPoGate("t_"+ to_string(i));   
    	CirGate* aiGate=ckt1_->getGateByName(_patchName[_patchName.size()-1]); 
    
    	ckt1_->connectPo(poGate, aiGate); 

      
       
        if (NoTime==false){     ckt1_->optimize_itp();}
        else  {ckt1_->optimize_itphalf();}

       
        ckt1_->outputPatch();

        cout<<"Best Cost: "<<BestCost<<endl;    
        cout<<"Patch Size: "<<ckt1_->output.size()+ckt1_->wire.size()<<endl;      
            

}

bool SATMgr::checkEq(vector<CirMgr*>& _cirVec){
   resetsolver();
   
  int cur_clauseNum = getNumClauses();
 
  _cirVec[0]->addPoCNF(_minisatPtr);
                                                                          
  Var* _on_OR_i_Var = new Var[ (_cirVec[0]->poneedlist.size() )/2];                        
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity((_cirVec[0]->poneedlist.size() )/2+ 1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (size_t j=0, n =(_cirVec[0]->poneedlist.size() )/2; j < n; ++j){   
     int i= _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j]);           //cout<<_cirVec[0]->poneedlist[j]->getName();
     int k=  _cirVec[0]->getPoIdx(_cirVec[0]->poneedlist[j+n]);        //cout<<_cirVec[0]->poneedlist[j+n]->getName();
    _on_OR_i_Var[j] = _minisatPtr->newVar();                             

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[j], _cirVec[0]->getPoVar(i) , false,			       _cirVec[0]->getPoVar(k) , false);                                                    
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                   
                                                                                                 
  // bind primary input
 /* for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }*/
                                                                                               
  resizeMarkVec(); 
  _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;

   return _minisatPtr->solve();
 
}

void SATMgr::buildforcheckEq(){
  assert(_isSingle);
  //_checkCir.push_back(ckt0_->duplicateS());
  _checkCir.clear();
  _checkCir.push_back(ckt1_->duplicate());
  
  
}


void SATMgr::buildSingle(){  
  assert(_isSingle);   _onSetCir.clear();_offSetCir.clear();
  //_onSetCir.push_back(ckt0_->duplicateSelect());
  _onSetCir.push_back(ckt1_->duplicateSelect());  
  buildONOFFset(_onSetCir, true, 0); // on set
  // _offSetCir.push_back(ckt0_->duplicateSelect()); 
  _offSetCir.push_back(ckt1_->duplicateSelect()); 
  buildONOFFset(_offSetCir, false, 0); // off set
  addComControl(_onSetCir[0], _offSetCir[0]);
}




void SATMgr::buildSingleSS(){  
  assert(_isSingle);    _onSetCir.clear();_offSetCir.clear();
  //_onSetCir.push_back(ckt0_->duplicateSelect());
  _onSetCir.push_back(ckt1_->duplicateSelect());  
  buildONOFFsetSS(_onSetCir, true, 0); // on set
  // _offSetCir.push_back(ckt0_->duplicateSelect()); 
  _offSetCir.push_back(ckt1_->duplicateSelect()); 
  buildONOFFsetSS(_offSetCir, false, 0); // off set
  addComControl(_onSetCir[0], _offSetCir[0]);
}



void SATMgr::buildSingle2(){  
  assert(_isSingle);         _onSetCir.clear();_offSetCir.clear();
  //_onSetCir.push_back(ckt0_->duplicateSelect());
  _onSetCir.push_back(ckt1_->duplicateSelect());
  _onSetCir[0]->Cut=commonVar;
  buildONOFFsetforItp(_onSetCir, true, 0); // on set
   //_offSetCir.push_back(ckt0_->duplicateSelect()); 
  _offSetCir.push_back(ckt1_->duplicateSelect()); 
  _offSetCir[0]->Cut=commonVar;
  buildONOFFsetforItp(_offSetCir, false, 0); // off set
  
}

bool SATMgr::myComparison1(const pair<vector<unsigned>,string> &a,const pair<vector<unsigned>,string> &b){
            if (a.first[0]!=b.first[0]){ 
                    return a.first[0]<b.first[0];
            }
                          
            else {
                    return a.second<b.second;
            }

}


bool SATMgr::myComparison2(const pair<vector<unsigned>,string> &a,const pair<vector<unsigned>,string> &b){
            if (a.first[0]!=b.first[0]){ 
                     return a.first[0]<b.first[0];
            }
                              
            else  {
                     if (a.first[1]!=b.first[1]){
                         return a.first[1]>b.first[1];
                     }
                     else {
                         return a.second>b.second;
                     }
            }


}




void SATMgr::buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit){

  int cur_clauseNum = getNumClauses();  
  _cirVec[0]->addCircuitCNF(_minisatPtr);                          //cout<<_cirVec[0]->poneedlist.size() /2;
                                                                          
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
                                                                                                 
 
                                                                                               
  resizeMarkVec(); 

  if (ON_OFF){ // on set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), true));        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set     
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);  

    }
  }
  else{ // off set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), false));
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

  
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
}






void SATMgr::buildONOFFsetSS(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit){
	//SatSolver* _solver = _solMgr._minisatPtr;
  
  int cur_clauseNum = getNumClauses();  
   
  _cirVec[0]->buildTopoList2();
  _cirVec[0]->addCircuitCNF(_minisatPtr);                          //cout<<_cirVec[0]->poneedlist.size() /2;
                                                                          
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
                                                                                                 
  // bind primary input
 /* for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }*/
                                                                                               
  resizeMarkVec(); 

  if (ON_OFF){ // on set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), true));        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set     
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);  

    }
  }
  else{ // off set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), false));
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

  
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
}




void SATMgr::buildONOFFsetforItp(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit){
	
  int cur_clauseNum = getNumClauses();  
  _cirVec[0]->addPoCNF(_minisatPtr);                          //cout<<_cirVec[0]->poneedlist.size() /2;
                                                                          
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
                                                                                                 
  // bind primary input
 /* for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }*/
                                                                                               
  resizeMarkVec(); 

  if (ON_OFF){ // on set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), true));        
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set     
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOnSet(i);  

    }
  }
  else{ // off set
    _minisatPtr->addClause(mkLit(_cirVec[0]->getWtVar(bit), false));
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=cur_clauseNum, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }
  }

  
	if(!_minisatPtr->simplify())
		cout << "ERROR in simplify" << endl;
}




void SATMgr::mapVar2Name(const Var& var, const string& name){
	assert(_var2Name.find(var)==_var2Name.end());
	_var2Name[var] = name;
}

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



void SATMgr::resizeMarkVec(){
	if (_isClauseOn.size() < getNumClauses())
		_isClauseOn.resize(getNumClauses(), false);
}


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
                     /*nId = ~_ntk->createNet();
                     createV3AndGate(_ntk, nId, ~nId1, ~nId2);
                     nId1 = nId;*/
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
                     /*nId = ~_ntk->createNet();
                     createV3AndGate(_ntk, nId, ~nId1, ~nId2);
                     nId1 = nId;*/
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
                     /*nId = _ntk->createNet();
                     createV3AndGate(_ntk, nId, nId1, nId2);
                     nId1 = nId;*/
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

   return nId;
}

