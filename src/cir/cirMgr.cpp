/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Circuit manager class ]
  Author       [ sam031023 ]
****************************************************************************/

#include "cir/cirMgr.h"

#include <iostream>
#include <cassert>

/**************************************
    Class CirMgr member functions
**************************************/
// Constructor
CirMgr::CirMgr(const std::string& name): name_(name), maxId_(0), const0_(NULL) {
  init();
}

// Destructor
CirMgr::~CirMgr() {
  if (const0_ != NULL) delete const0_;
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    delete gateList_[i];
  for(unsigned i = 0, n = fecGrpList_.size(); i < n; ++i)
    delete fecGrpList_[i];
}

// initialize
void CirMgr::init() {
  // delete old things
  if(const0_ != NULL) delete const0_;
  for(unsigned i = 0, n = gateList_.size(); i < n; ++i)
    delete gateList_[i];
  // initialize
  gateList_.clear();
  piList_.clear();
  poList_.clear();
  aigList_.clear();
  tList_.clear();
  name2GateMap_.clear();
}

// build AIG list
void CirMgr::buildAigList() {
  aigList_.clear();
  for(unsigned i = 0, n = gateList_.size(); i < n; ++i) {
    if(gateList_[i]->getType() == GATE_AIG)
      aigList_.push_back(gateList_[i]);
  }
}

// build topological list
void CirMgr::buildTopoList() {
  CirGate::incFlg();
  // reset topoList and used
  topoList_.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // always add const 0
  const0_->setToFlg();
  const0_->setUsed(true);
  topoList_.push_back(const0_);
  // DFS
  for (unsigned i = 0, n = poList_.size(); i < n; ++i) {       
    buildTopoList_rec(poList_[i]);
  }   
    for (unsigned i = 0, n = aigList_.size(); i < n; ++i) {     
    buildTopoList_rec(aigList_[i]);
  }
      
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    buildTopoList_rec(piList_[i]);
  }
  
}


void CirMgr::buildTopoList2() {
  CirGate::incFlg();
  // reset topoList and used
  topoList_.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // always add const 0
  const0_->setToFlg();
  const0_->setUsed(true);
  topoList_.push_back(const0_);
  // DFS
  for (unsigned i = 0, n = poList_.size(); i < n; ++i) {
    buildTopoList_rec(poList_[i]);
  }
   
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    buildTopoList_rec(piList_[i]);
  }
  
}



void CirMgr::buildTopoList3() {
  CirGate::incFlg();
  // reset topoList and used
  topoList_.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // always add const 0
  const0_->setToFlg();
  const0_->setUsed(true);
  topoList_.push_back(const0_);
  // DFS
  for (unsigned i = 0, n = poneedlist.size(); i < n; ++i) {
    buildTopoList_rec(poneedlist[i]);
  }
   
  for (unsigned i = 0, n = piList_.size(); i < n; ++i) {
    buildTopoList_rec(piList_[i]);
  }
  
}





void CirMgr::buildTopoList_rec(CirGate* gate) {                     
  if (gate->isFlg()) return;                                            
  gate->setToFlg();                                                 
  CirGateV in0 = gate->getFanin0();
  CirGateV in1 = gate->getFanin1();                              
  if (!in0.isNull() && !in0.isFloat()){
    buildTopoList_rec(in0.gate());
  }
  if (!in1.isNull() && !in1.isFloat()){
    buildTopoList_rec(in1.gate());
  }
  // set used and add to topoList
  gate->setUsed(true);  
  topoList_.push_back(gate);
}










void CirMgr::markT0Fanout(){
  for (unsigned i=0, n = tList_.size(); i<n; ++i){
    tList_[i]->_setTFanout(true);
  }
  for (unsigned i=0, n = topoList_.size(); i<n; ++i){
    CirGate* _gate = topoList_[i];
    if (_gate->getType()!=GATE_PI){
      if (_gate->_is_fanIn_in_t())
	  _gate->_setTFanout(true);
    }
  }
}

void CirMgr::buildBFSList(){
	assert(topoList_.size()!=0);
        CirGate::incFlg();
	GateList _level_zero_list;
	_BFS_Level_Vec.push_back(_level_zero_list);

        for (unsigned i=0, n=topoList_.size(); i<n; ++i){
            CirGate* _gate = topoList_[i];
            _gate->setToFlg();
            if (_gate->getType()==GATE_PI){
                   _gate->_setBFS_flag(0);
                   _BFS_Level_Vec[0].push_back(_gate);
      
            }
            else{
		   CirGate* _fanin1 = _gate->getFanin0().gate();
		   CirGate* _fanin2 = _gate->getFanin1().gate();

                   int _L1, _L2, _level;

                   if (_fanin1 != NULL)	{
			if (_fanin2 != NULL){
                             _level = (_fanin1->_get_BFS_flag() > _fanin2->_get_BFS_flag()? _fanin2->_get_BFS_flag()+1 : _fanin1->_get_BFS_flag()+1 );
			}
			else{
                             _level = _fanin1->_get_BFS_flag() +1;
			}
                   }
                   else if (_fanin2 != NULL){
                        _level = _fanin2->_get_BFS_flag() +1;
                   }
                   else{
                        _level = 0;
                   }
                   _gate->_setBFS_flag(_level);

                   if (_level > _BFS_Level_Vec.size()-1){
				GateList _newList;
                                _BFS_Level_Vec.push_back(_newList);
                   }
                   _BFS_Level_Vec[_level].push_back(_gate);
            }

        }

}


//select the PO containing target node
void CirMgr::selectPo(){
    
    CirGate::incFlg();
    poneedlist.clear();
    if (tList_.size()>0){
    	for (int i=0;i<tList_.size();i++){
                
        	
        	CirGate * tn=tList_[i];                 
        	selectPorec(tn);           
           
                
    	}
       
        
    	
    }
}
void CirMgr::selectPorec(CirGate* t0){
   if (t0->isFlg()){return;}
   t0->setSelect(true);
   t0->setpoNeed();     
   GateVList out=t0->getFanout();
  
   for (int i=0;i<out.size();i++){
       
       if(out[i].gate()->isFlg()){continue;}
       out[i].gate()->setSelect(true);
       out[i].gate()->setpoNeed();            
       if ((out[i].gate())->getType()==2 ){
             out[i].gate()->setToFlg();
          
             poneedlist.push_back(out[i].gate());
             poNeedNumlist.push_back(getPoIdx(out[i].gate()));
       }
       else {
             
             selectPorec(out[i].gate());
             
             out[i].gate()->setToFlg();
       }
       
   }
}


void CirMgr::findSize(){
      CirGate::incFlg();
      patch_size=0;
      findSizerec(poList_[poList_.size()-1]);
      cout<<"patch size: "<<patch_size<<endl;
}

void CirMgr::findSizerec(CirGate* _g){
  assert(_g!=NULL);
  if (_g->isFlg())	return;
  if (_g->getWeight()==-1){patch_size++;}
  else {return;}
  CirGate* _fanIn = _g->getFanin0().gate();
 
  if (_fanIn!=NULL){                   
	if (!_fanIn->isFlg()){  
		if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
			 
		}
		else{
			findSizerec(_fanIn);
			_fanIn->setToFlg();
		}
        }
  }

  _fanIn = _g->getFanin1().gate();
  if (_fanIn!=NULL){                       
	if (!_fanIn->isFlg()){ 
		if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
			 
		}
		else{
			findSizerec(_fanIn);
			_fanIn->setToFlg();
		}
        }
  }
}


void CirMgr::selectPi(){
	if (tList_.size()!=0){
		assert(poNeedNumlist.size()!=0);
        }
        CirGate::incFlg();
        piNeedList.clear();
        for (unsigned i=0, n=poNeedNumlist.size(); i<n; ++i){
               selectPiRec(poList_[poNeedNumlist[i]]);
        }
}

void CirMgr::selectPiRec(CirGate* _g){
  assert(_g!=NULL);
  if (_g->isFlg())	return;
  _g->setSelect(true);
  CirGate* _fanIn = _g->getFanin0().gate();
 
  if (_fanIn!=NULL){                   
	if (!_fanIn->isFlg()){  _fanIn->setSelect(true);
		if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
			piNeedList.push_back(_fanIn); 
		}
		else{
			selectPiRec(_fanIn);
			_fanIn->setToFlg();
		}
         }
  }

  _fanIn = _g->getFanin1().gate();
  if (_fanIn!=NULL){                       
	if (!_fanIn->isFlg()){  _fanIn->setSelect(true);
		if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
			piNeedList.push_back(_fanIn); 
		}
		else{
			selectPiRec(_fanIn);
			_fanIn->setToFlg();
		}
        }
  }
}


void CirMgr::selectConst(){


    CirGate::incFlg();
    selectGaterec(const0_,0) ;
    cout<<const0_->getName();
}

void CirMgr::selectGate(){
    
    CirGate::incFlg();
    
       
    if (piNeedList.size()>0){
    	for (int i=0;i<piNeedList.size();i++){
                levelCount=0;
        	
        	CirGate * tn=piNeedList[i];           	
        	selectGaterec(tn,levelCount);            
               
                
    	}
      
    }
    
}



void CirMgr::selectGaterec(CirGate* t0,unsigned levelcount){
   if (t0->isFlg()){return;}
   t0->setSelect(true);       
   if (levelcount>t0->getLevel()){t0->setLevel(levelcount);}
   GateVList out=t0->getFanout();
  
   for (int i=0;i<out.size();i++){
       
       if(out[i].gate()->isFlg()){continue;}
       
            
       selectGaterec(out[i].gate(),levelcount+1);      
       out[i].gate()->setToFlg();       
       
       
   }
}




void CirMgr::selectOther(){
	
  CirGate::incFlg();
  
  for (unsigned i=0, n=aigList_.size(); i<n; ++i){
    if (aigList_[i]->getSelect()){
        selectOtherRec(aigList_[i]);
     }
   }
}


void CirMgr::selectOtherRec(CirGate* _g){
  assert(_g!=NULL);
  if (_g->isFlg()){	return;}
  if (!_g->getSelect()){_g->setOther();}
  _g->setSelect(true);
  CirGate* _fanIn = _g->getFanin0().gate();
 
  if (_fanIn!=NULL){                   
	if (!_fanIn->isFlg()){  
              if (!_fanIn->getSelect()){_fanIn->setOther();}  
              _fanIn->setSelect(true);
	      if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
				
	      }
	      else{
			selectOtherRec(_fanIn);
			_fanIn->setToFlg();
             }
        }
  }

  _fanIn = _g->getFanin1().gate();
  if (_fanIn!=NULL){                       
	 if (!_fanIn->isFlg()){  
              if (!_fanIn->getSelect()){_fanIn->setOther();}  
              _fanIn->setSelect(true);
	      if (_fanIn->getType()==GATE_PI ){
			_fanIn->setToFlg();
		 
	      }
	      else{
			selectOtherRec(_fanIn);
			_fanIn->setToFlg();
	      }
          }
  }
}








void CirMgr::removeGate(){
    
    CirGate::incFlg();
    
    if (piList_.size()>0){
    	for (int i=0;i<piList_.size();i++){
                
        	
        	CirGate * tn=piList_[i];  
                if (!tn->getSelect()){         	
        	    removeGaterec(tn);            
                }
                
    	}     
    	
    }
   
    
}
void CirMgr::removeGaterec(CirGate* t0){
   if (t0->isFlg()){return;}
   
   t0->setSelect(false);
   GateVList out=t0->getFanout();
  
   for (int i=0;i<out.size();i++){
       
       if(out[i].gate()->isFlg()){continue;}
       
       
       removeGaterec(out[i].gate());
       out[i].gate()->setToFlg();       
       
       
   }
}




// build topolist for selected PO
void CirMgr::buildSelectPoTopoList() {
  CirGate::incFlg();
  // reset topoList and used
  SelectPotopoList_.clear();
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // always add const 0
  const0_->setToFlg();
  const0_->setUsed(true);
  SelectPotopoList_.push_back(const0_);
  // DFS           
   
  for (unsigned i = 0, n =poneedlist.size(); i < n; ++i) {
    
    buildSelectPoTopoList_rec(poneedlist[i] );
  }
  for (unsigned i = 0, n =Cut.size(); i < n; ++i) {
    CirGate* gate=getGateByName(Cut[i]);
    buildSelectPoTopoList_rec(gate);
  }


}

//build topolist for selected PO recursively
void CirMgr::buildSelectPoTopoList_rec(CirGate* gate) {
  if (gate->isFlg()) return;
  gate->setToFlg();
  CirGateV in0 = gate->getFanin0();
  CirGateV in1 = gate->getFanin1();
  if (!in0.isNull() && !in0.isFloat())
    buildSelectPoTopoList_rec(in0.gate());
  if (!in1.isNull() && !in1.isFloat())
    buildSelectPoTopoList_rec(in1.gate());
  // set used and add to topoList
  gate->setUsed(true);  
  SelectPotopoList_.push_back(gate);
}













// build unate array
void CirMgr::buildUnateArr() {
  unsigned n = getPiNum();
  unsigned m = getPoNum();
  // resize the array
  unateArr_.resize(n);
  for (unsigned i = 0; i < n; ++i)
    unateArr_[i].resize(m);
  // fill the array
  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = 0; j < n; ++j)
      piList_[j]->setUnateness(j == i? Unateness(1) : Unateness(0));
    for (unsigned j = 0; j < topoList_.size(); ++j)
      topoList_[j]->simUnate();
    for (unsigned j = 0; j < m; ++j)
      unateArr_[i][j] = poList_[j]->getUnateness();
  }
}



// duplicate a circuit
CirMgr* CirMgr::duplicate() const {
  CirMgr* newMgr = new CirMgr;
  std::map<CirGate*, CirGate*> gateMap;
  gateMap[0] = 0;
  // set name
  newMgr->setName(getName());
  // add gate and set gate mapping
  gateMap[getConstGate()] = newMgr->addConstGate();
  for (unsigned i = 0, n = getPiNum(); i < n; ++i)
    gateMap[getPiGate(i)] = newMgr->addPiGate(getPiName(i));
  for (unsigned i = 0, n = getPoNum(); i < n; ++i){
    gateMap[getPoGate(i)] = newMgr->addPoGate(getPoName(i));
    if (getPoGate(i)->getpoNeed()){    
              
        newMgr->poneedlist.push_back(gateMap[getPoGate(i)]);
    } 
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i)
    gateMap[getAIGate(i)] = newMgr->addAIGate(getAigName(i));
  for (unsigned i = 0, n = getWtNum(); i < n; ++i)
    gateMap[getWtGate(i)] = newMgr->addWtGate(getWtName(i));

  // connect gates
  for (unsigned i = 0, n = getPoNum(); i < n; ++i) {
    CirGate* gate = getPoGate(i);
    assert(gate != NULL);
    CirGate* out = gateMap[gate];
    CirGate* in = gateMap[gate->getFanin0().gate()];
    bool inv = gate->getFanin0().isInv();
    out->setFanin(CirGateV(in, inv));
    in->addFanout(CirGateV(out, inv));
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i) {
    CirGate* gate = getAIGate(i);
    assert(gate != NULL);
    CirGate* out = gateMap[gate];
    CirGate* in0 = gateMap[gate->getFanin0().gate()];
    bool inv0 = gate->getFanin0().isInv();
    CirGate* in1 = gateMap[gate->getFanin1().gate()];
    bool inv1 = gate->getFanin1().isInv();
    out->setFanin(CirGateV(in0, inv0), CirGateV(in1, inv1));
    if (in0) in0->addFanout(CirGateV(out, inv0));
    if (in1) in1->addFanout(CirGateV(out, inv1));
  }
  
  newMgr->buildAigList();
  newMgr->buildTopoList();
  
  return newMgr;
}





// duplicate the gates we need only
CirMgr* CirMgr::duplicateSelect() const {
  CirMgr* newMgr = new CirMgr;
  std::map<CirGate*, CirGate*> gateMap;
  gateMap[0] = 0;
  // set name
  newMgr->setName(getName());
  // add gate and set gate mapping
  gateMap[getConstGate()] = newMgr->addConstGate();
  for (unsigned i = 0, n = getPiNum(); i < n; ++i){
    if (!getPiGate(i)->getSelect()){continue;}
    gateMap[getPiGate(i)] = newMgr->addPiGate(getPiName(i));
  }
  for (unsigned i = 0, n = getPoNum(); i < n; ++i){
    if (!getPoGate(i)->getSelect()){continue;}
    gateMap[getPoGate(i)] = newMgr->addPoGate(getPoName(i));
    if (getPoGate(i)->getpoNeed()){    
             
        newMgr->poneedlist.push_back(gateMap[getPoGate(i)]);
    } 
   
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i){
    if (!getAIGate(i)->getSelect() ){continue;}
    gateMap[getAIGate(i)] = newMgr->addAIGate(getAigName(i));
  }
  for (unsigned i = 0, n = getWtNum(); i < n; ++i)
    gateMap[getWtGate(i)] = newMgr->addWtGate(getWtName(i));

  // connect gates
  for (unsigned i = 0, n = getPoNum(); i < n; ++i) {
    if (!getPoGate(i)->getSelect()){continue;}
    CirGate* gate = getPoGate(i);
    assert(gate != NULL);
    CirGate* out = gateMap[gate];
    CirGate* in = gateMap[gate->getFanin0().gate()];
    bool inv = gate->getFanin0().isInv();
    out->setFanin(CirGateV(in, inv));
    in->addFanout(CirGateV(out, inv));
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i) {
    if (!getAIGate(i)->getSelect()){continue;}
    CirGate* gate = getAIGate(i);
    assert(gate != NULL);
    CirGate* out = gateMap[gate];
    CirGate* in0 = gateMap[gate->getFanin0().gate()];
    bool inv0 = gate->getFanin0().isInv();
    CirGate* in1 = gateMap[gate->getFanin1().gate()];
    bool inv1 = gate->getFanin1().isInv();
    out->setFanin(CirGateV(in0, inv0), CirGateV(in1, inv1));
    if (in0) in0->addFanout(CirGateV(out, inv0));
    if (in1) in1->addFanout(CirGateV(out, inv1));
  }
  
  newMgr->buildAigList();
  newMgr->buildTopoList();
  newMgr->buildBFSList();
  return newMgr;
}




// duplicate a circuit
void CirMgr::duplicateGold(CirMgr* ckt1) const {
  
  std::map<CirGate*, CirGate*> gateMap;
  gateMap[0] = 0;                 
  // set name
  //newMgr->setName(getName());
  // add gate and set gate mapping
  
  gateMap[getConstGate()] = ckt1->getConstGate();
  for (unsigned i = 0, n = getPiNum(); i < n; ++i){
    if (!getPiGate(i)->getSelect()){continue;}
    string name=getPiName(i);
    gateMap[getPiGate(i)] = ckt1->getGateByName(name);
  }
  for (unsigned i = 0, n = getPoNum(); i < n; ++i){
    if (!getPoGate(i)->getSelect()){continue;}
    gateMap[getPoGate(i)] = ckt1->addPoGate("G"+getPoName(i));
    gateMap[getPoGate(i)]->setSelect(true);
              
    ckt1->poneedlist.push_back(gateMap[getPoGate(i)]);
    gateMap[getPoGate(i)]->setpoNeed();
     
   
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i){
    if (!getAIGate(i)->getSelect() ){continue;}
    gateMap[getAIGate(i)] = ckt1->addAIGate("G"+getAigName(i));
    gateMap[getAIGate(i)]->setSelect(true);
  }
  //for (unsigned i = 0, n = getWtNum(); i < n; ++i)
  // gateMap[getWtGate(i)] = newMgr->addWtGate(getWtName(i));

  // connect gates
  for (unsigned i = 0, n = getPoNum(); i < n; ++i) {
    if (!getPoGate(i)->getSelect()){continue;}
    CirGate* gate = getPoGate(i);
    assert(gate != NULL);    
    CirGate* out = gateMap[gate];                                
    CirGate* in = gateMap[gate->getFanin0().gate()];             
    bool inv = gate->getFanin0().isInv();
    out->setFanin(CirGateV(in, inv));
    in->addFanout(CirGateV(out, inv));
  }
  for (unsigned i = 0, n = getAigNum(); i < n; ++i) {
    if (!getAIGate(i)->getSelect()){continue;}
    CirGate* gate = getAIGate(i);
    assert(gate != NULL);
    CirGate* out = gateMap[gate];
    CirGate* in0 = gateMap[gate->getFanin0().gate()];
    bool inv0 = gate->getFanin0().isInv();
    CirGate* in1 = gateMap[gate->getFanin1().gate()];
    bool inv1 = gate->getFanin1().isInv();
    out->setFanin(CirGateV(in0, inv0), CirGateV(in1, inv1));
    if (in0) in0->addFanout(CirGateV(out, inv0));
    if (in1) in1->addFanout(CirGateV(out, inv1));
  }
  
  ckt1->buildAigList();
  ckt1->buildTopoList();
  //ckt1->buildBFSList();
  
}





// get index of a gate in PI, return -1 if not found
int CirMgr::getPiIdx(const CirGate* gate) const {
  for (unsigned i = 0, n = piList_.size(); i < n; ++i)
    if (piList_[i] == gate)
      return i;
  return -1;
}

// get index of a gate in PO, return -1 if not found
int CirMgr::getPoIdx(const CirGate* gate) const {
  for (unsigned i = 0, n = poList_.size(); i < n; ++i)
    if (poList_[i] == gate)
      return i;
  return -1;
}

// get whether a gate is used
bool CirMgr::getGateUsed(CirGate* gate) const {
  return gate->getUsed();
}

// get whether a PI is used
bool CirMgr::getPiUsed(int id) const {
  return getGateUsed(piList_[id]);
}

// get used PI vector
std::vector<bool> CirMgr::getPiUsed() const {
  unsigned nPi = piList_.size();
  unsigned nPo = poList_.size();
  std::vector<bool> ret(nPi);
  CirGate::incFlg();
  // reset used
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // DFS
  for (unsigned i = 0; i < nPo; ++i)
    dfs_backward(poList_[i]);
  // get used gates
  for (unsigned i = 0; i < nPi; ++i)
    ret[i] = piList_[i]->getUsed();
  return ret;
}

// get fanin cone size of a gate
unsigned CirMgr::getGateFaninSize(CirGate* gate) const {
  unsigned ret = 0;
  CirGate::incFlg();
  // reset and used
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // DFS
  dfs_backward(gate);
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    if(gateList_[i]->getUsed()) ret += 1;
  return ret;
}

// get fanin cone size of a gate
unsigned CirMgr::getGateFaninPiSize(CirGate* gate) const {
  unsigned ret = 0;
  CirGate::incFlg();
  // reset and used
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // DFS
  dfs_backward(gate);
  for (unsigned i = 0, n = piList_.size(); i < n; ++i)
    if(piList_[i]->getUsed()) ret += 1;
  return ret;
}

// get fanin cone size of idx-th PO
unsigned CirMgr::getPoFaninSize(int idx) const {
  return getGateFaninSize(poList_[idx]);
}

// get fanin cone size of idx-th PO
unsigned CirMgr::getPoFaninPiSize(int idx) const {
  return getGateFaninPiSize(poList_[idx]);
}

// get fanin cone size of all POs
std::vector<unsigned> CirMgr::getPoFaninSizeVec() const {
  std::vector<unsigned> ret(poList_.size());
  for (unsigned i = 0, n = poList_.size(); i < n; ++i)
    ret[i] = getPoFaninSize(i);
  return ret;
}

// get fanout cone size of a gate
unsigned CirMgr::getGateFanoutSize(CirGate* gate) const {
  unsigned ret = 0;
  CirGate::incFlg();
  // reset and used
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    gateList_[i]->setUsed(false);
  // DFS
  dfs_forward(gate);
  for (unsigned i = 0, n = gateList_.size(); i < n; ++i)
    if (gateList_[i]->getUsed()) ret += 1;
  return ret;
}

// get fanout cone size of idx-th PI
unsigned CirMgr::getPiFanoutSize(int idx) const {
  return getGateFanoutSize(piList_[idx]);
}

// get fanout cone size of all PIs
std::vector<unsigned> CirMgr::getPiFanoutSizeVec() const {
  std::vector<unsigned> ret(piList_.size());
  for (unsigned i = 0, n = piList_.size(); i < n; ++i)
    ret[i] = getPiFanoutSize(i);
  return ret;
}

// get equal POs
std::vector<unsigned> CirMgr::getEqPo(unsigned id) const {
  std::vector<unsigned> ret;
  for (unsigned i = 0, n = poList_.size(); i < n; ++i) {
    if (poList_[i]->getFanin0() == poList_[id]->getFanin0())
      ret.push_back(2*i);
    if (poList_[i]->getFanin0() == ~(poList_[id]->getFanin0()))
      ret.push_back(2*i+1);
  }
  return ret;
}

void CirMgr::dfs_backward(CirGate* gate) const {
  if(gate->isFlg()) return;
  gate->setToFlg();
  CirGateV in0 = gate->getFanin0();
  CirGateV in1 = gate->getFanin1();
  if (!in0.isNull() && !in0.isFloat())
    dfs_backward(in0.gate());
  if (!in1.isNull() && !in1.isFloat())
    dfs_backward(in1.gate());
  // set used
  gate->setUsed(true);
}

void CirMgr::dfs_forward(CirGate* gate) const {
  if (gate->isFlg()) return;
  gate->setToFlg();
  GateVList fanout = gate->getFanout();
  for (unsigned i = 0, n = fanout.size(); i < n; ++i)
    dfs_forward(fanout[i].gate());
  // set used
  gate->setUsed(true);
}

/*
// get symmetric PI groups
std::vector<std::vector<unsigned> > CirMgr::getSymmetricPiGrps() {
  unsigned nPi = piList_.size();
  std::vector<std::vector<bool> > posSym(nPi, std::vector<bool>(nPi, true));
  std::vector<std::vector<bool> > negSym(nPi, std::vector<bool>(nPi, true));
  std::vector<bool> visit(nPi);
  std::vector<std::vector<unsigned> > symGrps;
  // init solver
  SatSolver solver;
  std::vector<Var> piVar0, piVar1, ctrl;
  buildSymmetryMiter(&solver, &piVar0, &piVar1, &ctrl);
  symGrps.push_back(std::vector<unsigned>(0));
  for (unsigned i = 0; i < nPi; ++i)
    visit[i] = (!piList_[i]->getUsed());
  // find groups
  for (unsigned i = 0; i < nPi; ++i) {
    if (visit[i]) continue;
    visit[i] = true;
    std::vector<unsigned> grp;
    grp.push_back(2*i);
    for (unsigned j = i+1; j < nPi; ++j) {
      if (visit[j]) continue;
      if (posSym[i][j]) {
        if (provePiSymmetry(&solver, piVar0, piVar1, ctrl, i, j, false)) {
          grp.push_back(2*j);
          visit[j] = true;
          negSym[i][j] = false;
        } else {
          std::vector<bool> P(nPi);
          for (unsigned k = 0; k < nPi; ++k)
            P[k] = solver.getVal(piVar0[k]);
          P[i] = !P[i];
          simPiSymmetry(P, &posSym, &negSym);
        }
      }
      if(negSym[i][j]) {
        if (provePiSymmetry(&solver, piVar0, piVar1, ctrl, i, j, true)) {
          grp.push_back(2*j+1);
          visit[j] = true;
          posSym[i][j] = false;
        } else {
          std::vector<bool> P(nPi);
          for (unsigned k = 0; k < nPi; ++k)
            P[k] = solver.getVal(piVar0[k]);
          P[i] = !P[i];
          simPiSymmetry(P, &posSym, &negSym);
        }
      }
    }
    if (grp.size() > 1) {
      symGrps.push_back(grp);
    } else {
      symGrps[0].push_back(2*i);
    }
  }
#ifdef VERBOSE
  std::cout << "sym groups:" << std::endl;
  for (unsigned i = 0; i < symGrps.size(); ++i) {
    std::vector<unsigned>& grp = symGrps[i];
    for (unsigned j = 0; j < grp.size(); ++j)
      std::cout << ((grp[j]&1) ? "!" : "") << getPiName(grp[j]/2) << " ";
    std::cout << std::endl;
  }
#endif
  return symGrps;
}


*/

// find non-symmetry pair using simulation
// [ Symmetry Detection for Large Boolean Functions using
//   Circuit Representation, Simulation and Satisfiability ]
void CirMgr::simPiSymmetry(const std::vector<bool>& P,
                           std::vector<std::vector<bool> >* posSym,
                           std::vector<std::vector<bool> >* negSym) {
  unsigned nPi = piList_.size();
  unsigned nPo = poList_.size();
  std::vector<unsigned> S(nPi);
  std::vector<unsigned> R(nPi, 0u);
  std::vector<unsigned> A1(nPi, 0u);
  std::vector<unsigned> A2(nPi, 0u);
  std::vector<unsigned> B1(nPi, 0u);
  std::vector<unsigned> B2(nPi, 0u);
  for (unsigned i = 0; i < nPi; i += 32) {
    for (unsigned j = 0; j < nPi; ++j)
      S[j] = (P[j] ? ~0u : 0u);
    for (unsigned j = 0; j < 32 && i+j < nPi; ++j)
      S[i+j] ^= (1 << j);
    simulate(S);
    for (unsigned j = 0; j < 32 && i+j < nPi; ++j)
      for (unsigned k = 0; k < nPo; ++k)
        R[i+j] |= ((poList_[k]->getSimVal() >> j) & 1) << k;
  }
  for (unsigned i = 0; i < nPi; ++i) {
    unsigned extP = (P[i] ? ~0u : 0u);
    A1[i] =  extP &  R[i];
    A2[i] =  extP & ~R[i];
    B1[i] = ~extP &  R[i];
    B2[i] = ~extP & ~R[i];
  }
  for (unsigned i = 0; i < nPi; ++i) {
    for (unsigned j = 0; j < nPi; ++j) {
      if ((A1[i] & A2[j]) | (B1[i] & B2[j]))
        (*posSym)[i][j] = (*posSym)[j][i] = false;
      if ((A1[i] & B2[j]) | (B1[i] & A2[j]))
        (*negSym)[i][j] = (*negSym)[j][i] = false;
    }
  }
}

// build a miter to prove PI symmetry
void CirMgr::buildSymmetryMiter(SatSolver* solver,
                                std::vector<Var>* piVar0,
                                std::vector<Var>* piVar1,
                                std::vector<Var>* ctrl) {
  unsigned nPi = piList_.size();
  unsigned nPo = poList_.size();
  vec<Lit> clause;
  piVar0->resize(nPi);
  piVar1->resize(nPi);
  Var poVar0[nPo], poVar1[nPo];
  // build miter circuit CNF
  solver->init();
  addCircuitCNF(solver);
  for (unsigned i = 0; i < nPi; ++i) (*piVar0)[i] = getPiVar(i);
  for (unsigned i = 0; i < nPo; ++i) poVar0[i] = getPoVar(i);
  addCircuitCNF(solver);
  for (unsigned i = 0; i < nPi; ++i) (*piVar1)[i] = getPiVar(i);
  for (unsigned i = 0; i < nPo; ++i) poVar1[i] = getPoVar(i);
  // PO constraint
  for (unsigned i = 0; i < nPo; ++i) {
    Var dif = solver->newVar();
    solver->addXorCNF(dif, poVar0[i], false, poVar1[i], false);
    clause.push(mkLit(dif));
  }
  solver->addClause(clause);
  // PI constraint
  ctrl->resize(nPi);
  for (unsigned i = 0; i < nPi; ++i) {
    (*ctrl)[i] = solver->newVar();
    solver->addClause(~mkLit((*ctrl)[i]),
                       mkLit((*piVar0)[i]),
                      ~mkLit((*piVar1)[i]));
    solver->addClause(~mkLit((*ctrl)[i]),
                      ~mkLit((*piVar0)[i]),
                       mkLit((*piVar1)[i]));
  }
}


