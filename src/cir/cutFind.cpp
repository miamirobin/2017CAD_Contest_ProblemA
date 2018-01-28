/****************************************************************************
  FileName     [ cutFind.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Functions for finding cut ]
  Author       [ sam031023 ]
****************************************************************************/

#include <cassert>
#include <climits>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <algorithm>
#include <time.h>
//#include "util/parse.h"
#include "cir/satMgr.h"

using namespace std;


void SATMgr::sol_sin_plus(){
  assert(_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];
 
	for (int i=0, n=_weightOrder.size(); i<n; ++i){
		_assume.push(mkLit(_OrderedVar[i], true)); // 
                _common[i] =false;
	}
  while (true){ 

    
   
      for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		if ( !_minisatPtr->solve(_assume)){  
                     _common[_flag] = true; // make them be common variable
                      commonVar.push_back(_weightOrder[_flag].second); BestCost+=_weightOrder[_flag].first[0];
                      
                     break;
		}
                		
      }
    
      for (int _flag=0, n=_weightOrder.size(); _flag<n; ++_flag){
             
              if ( _common[_flag]){ 
		    break;
               }
               else {
                  _assume[_flag] = mkLit(_OrderedVar[_flag], true);
               }
      }
    
       if ( !_minisatPtr->solve(_assume)){  
      
               break;
       }

  }

  

  
  int _cost = 0;
  int _usedNet = 0;
  
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
    if (_common[i]){
			_cost += _weightOrder[i].first[0];  
			_usedNet++;
		
    }
  }

}






void SATMgr::sol_sin_minus(){
  assert(_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];

	for (int i=0, n=_weightOrder.size(); i<n; ++i){
		_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first
                _common[i] =false;
	}

  for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = true; // make them be common variable
                     break;
		}
                _common[_flag] = true;
		
  }

   for (int _flag = _weightOrder.size()-1; _flag>=0; --_flag){
		if (_common[_flag] ==false){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = false; // make them be free variable
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;            Cut.push_back(_weightOrder[_flag].second); Cost+=_weightOrder[_flag].first[0];
		}
                
		
  }


 

  
  int _cost = 0;
  int _usedNet = 0;
 
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
    if (_common[i]){
			_cost += _weightOrder[i].first[0];  
			_usedNet++;
			
    }
  }
 

   if(Cost>0 and (Cost<BestCost or BestCost==0) ){commonVar.clear();commonVar=Cut;BestCost=Cost;}
  Cut.clear();
  Cost=0;

}




void SATMgr::sol_sin_zero(){
  assert(_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];

	for (int i=0, n=_weightOrder.size(); i<n; ++i){                
		_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first
                _common[i] = false;
	}

  for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = true; // make them be common variable
                     break;
		}
                _common[_flag] = true;
		
  }


   for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		if (_common[_flag] ==false){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = false; // make them be free variable
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;    Cut.push_back(_weightOrder[_flag].second); Cost+=_weightOrder[_flag].first[0];
		}
                
		
  }
 


  
  
  int _cost = 0;
  int _usedNet = 0;
 
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
    if (_common[i]){
			_cost += _weightOrder[i].first[0];  
			_usedNet++;
			
    }
  }
  

  if(Cost>0 and (Cost<BestCost or BestCost==0)){commonVar.clear();commonVar=Cut;BestCost=Cost;}
  Cut.clear();
  Cost=0;
}




void SATMgr::sol_sin_one(){
	cout << "Find solution by Algorithm 1" << endl;
	assert((_isSingle));

	vec<Lit> _assume;
        string _singleNetName = "";
	unsigned _cost = 0;

	for (int i=0, n=_weightOrder.size(); i<n; ++i)
		_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first


   for (int _flag =0, n = _weightOrder.size()-1; _flag < n; ++_flag){
		// avoid using t_0~t_n 's fanout cone as common variable
		//CirGate* _curGate = ckt1_->getGateByName(_weightOrder[_flag].second);
		//if (_curGate->_is_t_Fanout())		continue;
                string name =_weightOrder[_flag].second; 
               // if (!ckt1_->getGateByName(name)->getPure()){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		//string _NETNAME =_weightOrder[_flag].second;
		//cout << "Try to use " << _NETNAME ;
		if ( !_minisatPtr->solve(_assume)){
			
                        _singleNetName = _weightOrder[_flag].second;
                        _cost = _weightOrder[_flag].first[0];
                        Cost += _weightOrder[_flag].first[0];
                        commonVar.clear();
                        commonVar.push_back(_singleNetName);
                        BestCost=Cost;
                        break;
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		}
   }

   Cost=0;
}
 
void SATMgr::sol_sin_two(){
  assert(_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];

	for (int i=0, n=_weightOrder.size(); i<n; ++i){
		_assume.push(mkLit(_OrderedVar[i], false)); // make all variable common at first
	}

  for (int _flag = _weightOrder.size()-1; _flag>=0; --_flag){
		// avoid using t_0~t_n 's fanout cone as common variable
		CirGate* _curGate = ckt1_->_getGateInAllMap(_weightOrder[_flag].second);
		

		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){
      _common[_flag] = false; // make them be free variable
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;  //commonVar.push_back(_weightOrder[_flag].second); BestCost+=_weightOrder[_flag].first;
		}
  }
  int _cost = 0;
  int _usedNet = 0;
  
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
    if (_common[i]){
	_cost += _weightOrder[i].first[0];  
	_usedNet++;
			
    }
  }
 
}

void SATMgr::sol_sin_three(){
  assert(_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];

	for (int i=0, n=_weightOrder.size(); i<n; ++i){
		_assume.push(mkLit(_OrderedVar[i], false)); // make all variable common at first
	}

  for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		// avoid using t_0~t_n 's fanout cone as common variable
		CirGate* _curGate = ckt1_->_getGateInAllMap(_weightOrder[_flag].second);
		

		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){
                        _common[_flag] = false;
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;
		}
  }
  int _cost = 0;
  int _usedNet = 0;
 
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
        if (_common[i]){
		_cost += _weightOrder[i].first[0];
		_usedNet++;
		
        }
  }
  
}

void SATMgr::sol_sin_four(){
  assert(_isSingle);
  vec<Lit> _assume;

  for (int i=0, n=_weightOrder.size(); i<n; ++i)
	_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first

  unsigned _totlaCost = UINT_MAX;
  unsigned _bestLevel = 0;
  
  for (int i=0, n = ckt1_->_BFS_Level_Vec.size(); i<n; ++i){
   
    unsigned m=ckt1_->_BFS_Level_Vec[i].size();
    unsigned _curCost = 0;
   
    map<string, Var>::iterator _it;
    for (int j=0 ; j<m; ++j){
      if (ckt1_->_BFS_Level_Vec[i][j]->_is_t_Fanout())	continue;

      string _gateName = ckt1_->_BFS_Level_Vec[i][j]->getName();
      _it = _gate2ConVar.find(_gateName);  
      if (_it != _gate2ConVar.end() ){

		_assume.push(mkLit(_it->second, false));
		_curCost += _weightMap.find(_gateName)->second;
      }
    }
   
    if ( !_minisatPtr->solve(_assume) ){
			
			if (_curCost < _totlaCost){
				_totlaCost = _curCost;
				_bestLevel = i;
			}
			
    }
    _assume.clear();


  }
	
}


void SATMgr::sol_sim_five(){
  unsigned _cost = 0;
  string _gateName;
  vec<Lit> _assume;
  for (unsigned i=0, n = ckt1_->piNeedList.size(); i<n; ++i){
		_gateName = ckt1_->piNeedList[i]->getName();
		_cost += _weightMap.find(_gateName)->second;
		_assume.push(mkLit(_gate2ConVar.find(_gateName)->second, false));
  }
  if (!_minisatPtr->solve(_assume) ){
   
  }
}



void SATMgr::sol_mul_one(){
  assert(!_isSingle);
  vec<Lit> _assume; 
  bool* _common = new bool[_weightOrder.size()];
       
  for (int i=0, n=_weightOrder.size(); i<n; ++i){  
	  _assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first
              
          _common[i] =false;
  }   

  for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){

           if (   (double)clock() / CLOCKS_PER_SEC  >300) {NoTime=true; return;}

		
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = true; // make them be common variable
                     break;
		}
                _common[_flag] = true;

               
           

		
  }

   for (int _flag = _weightOrder.size()-1; _flag>=0; --_flag){
		if (_common[_flag] ==false){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){        
                     _common[_flag] = false; // make them be free variable
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;    Cut.push_back(_weightOrder[_flag].second); Cost+=_weightOrder[_flag].first[0];
		}
                
		
  }


 

  
  int _cost = 0;
  int _usedNet = 0;
  
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
      if (_common[i]){
			_cost += _weightOrder[i].first[0];  
			_usedNet++;
			
      }
  }
 
  if(Cost>0 and (Cost<BestCost or BestCost==0) ){commonVar.clear();commonVar=Cut;BestCost=Cost;}
  Cut.clear();
  Cost=0;


}




void SATMgr::sol_mul_two(){
  assert(!_isSingle);
  vec<Lit> _assume;
  bool* _common = new bool[_weightOrder.size()];

	for (int i=0, n=_weightOrder.size(); i<n; ++i){
		_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first
                _common[i] = false;
	}

  for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = true; // make them be common variable
                     break;
		}
                _common[_flag] = true;
		
  }


   for (int _flag = 0, n = _weightOrder.size(); _flag < n; ++_flag){
		if (_common[_flag] ==false){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		if ( !_minisatPtr->solve(_assume)){
                     _common[_flag] = false; // make them be free variable
		}
		else{
			_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
			_common[_flag] = true;   Cut.push_back(_weightOrder[_flag].second); Cost+=_weightOrder[_flag].first[0];
		}
                
		
  }
 


  
  
  int _cost = 0;
  int _usedNet = 0;
  
  for (unsigned i=0, n = _weightOrder.size(); i<n; ++i){
    if (_common[i]){
			_cost += _weightOrder[i].first[0];  
			_usedNet++;
			
    }
  }
  

  if(Cost>0 and (Cost<BestCost or BestCost==0) ){commonVar.clear();commonVar=Cut;BestCost=Cost;}
  Cut.clear();
  Cost=0;



}


void SATMgr::sol_mul_zero(){
	cout << "Using Alg 0" << endl;
	assert((_isSingle));

	vec<Lit> _assume;
        string _singleNetName = "";
	unsigned _cost = 0;

	for (int i=0, n=_weightOrder.size(); i<n; ++i)
		_assume.push(mkLit(_OrderedVar[i], true)); // make all variable free at first


        for (int _flag =0, n = _weightOrder.size()-1; _flag < n; ++_flag){
		// avoid using t_0~t_n 's fanout cone as common variable
		//CirGate* _curGate = ckt1_->getGateByName(_weightOrder[_flag].second);
		//if (_curGate->_is_t_Fanout())		continue;
                string name =_weightOrder[_flag].second; 
               // if (!ckt1_->getGateByName(name)->getPure()){continue;}
		_assume[_flag] = mkLit(_OrderedVar[_flag], false); // make them be common variable
		//string _NETNAME =_weightOrder[_flag].second;
		//cout << "Try to use " << _NETNAME ;
		if ( !_minisatPtr->solve(_assume)){
			 
                      _singleNetName = _weightOrder[_flag].second;
                      _cost = _weightOrder[_flag].first[0];
                      Cost +=  _weightOrder[_flag].first[0];
                      commonVar.clear();
                      commonVar.push_back(_singleNetName);
                      BestCost=Cost;
                      break;
		}
		else{
		      _assume[_flag] = mkLit(_OrderedVar[_flag], true); // make them be free variable
		}
         }

	
         Cost=0;
}

