/****************************************************************************
  FileName     [ satMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ sam031023 ]
****************************************************************************/

#include <cassert>
#include <iostream>
#include <vector>
#include "Minisat/satMgr.h"

using namespace std;

void SATMgr::reset(){
	if (_minisatPtr)	delete _minisatPtr;
	_minisatPtr = new SatSolver();
	_minisatPtr->init();
}

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
		_isClauseOn.resize(_isClauseOn.size(), false);
}

void SATMgr::buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit){
	//SatSolver* _solver = _solMgr._minisatPtr;

  _cirVec[0]->addCircuitCNF(_minisatPtr); 
  _cirVec[1]->addCircuitCNF(_minisatPtr);

  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];
  Var  _on_OR_o_var = _minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)

  for (size_t i=0, n = _cirVec[0]->getPoNum(); i < n; ++i){
    _on_OR_i_Var[i] = _minisatPtr->newVar();

    // add output difference gate
    _minisatPtr->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false, 
                                        _cirVec[1]->getPoVar(i)   ,false);
    // add (~Input i + Output)
    _minisatPtr->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false)); 

    // (~Output += Input 0~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }
  _minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)

  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _minisatPtr->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }

  resizeMarkVec();

  if (ON_OFF){
  	_minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(bit),false));
  	_minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark on set
    for (size_t i=0, n = getNumClauses(); i < n; ++i){
      markOnSet(i);
    }
  }
  else{
    _minisatPtr->addClause(mkLit(_cirVec[1]->getWtVar(bit),true));
    _minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    // mark off set
    for (size_t i=0, n = getNumClauses(); i < n; ++i){
      markOffSet(i);
    }  
  }
  
  //delete []_on_OR_i_Var;
  //return _on_OR_o_var;

}
