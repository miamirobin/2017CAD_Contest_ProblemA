/****************************************************************************
  FileName     [ cirMiter.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Circuit miter class ]
  Author       [ sam031023 ]
****************************************************************************/

#include "cir/cirMiter.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <cassert>

/**************************************
    Class CirMiter member functions
**************************************/
// Constructor
/*
CirMiter::CirMiter(): ckt0_(NULL), ckt1_(NULL) {}

// Destructor
CirMiter::~CirMiter() {
  if (ckt0_ != NULL) delete ckt0_;
  if (ckt1_ != NULL) delete ckt1_;
}

// initialize
void CirMiter::init() {
  if (ckt0_ != NULL) delete ckt0_;
  if (ckt1_ != NULL) delete ckt1_;
  ckt0_ = ckt1_ = NULL;
}

// initialize solvers
void CirMiter::initSolver() {
  _miterSlover.reset();
  _miterSlover.ckt0_ = ckt0_;
  _miterSlover.ckt1_ = ckt1_;
}

void CirMiter::buildList(){
  assert(ckt0_);
  assert(ckt1_);
  ckt0_->buildAigList();
  ckt1_->buildAigList();
  ckt0_->buildTopoList();
  ckt1_->buildTopoList();
}

// read Verilog
void CirMiter::readVerilog(const string& filename0, const string& filename1) {
  init();
  ckt0_ = new CirMgr();
  ckt1_ = new CirMgr();
  if (!ckt0_->readVerilog(filename0) || !ckt1_->readVerilog(filename1)){
    cout << "ERROR:: parsing error!" << endl;
    return;
  }
	//buildList();
	if (ckt1_->tList_.size()==1)  _isSingle = true;
	else													_isSingle = false;
  
  _miterSlover.getItp();
}

void CirMiter::buildSingle(){
  buildCircuitSolver(true);
  if (_globalVerbosityFlg) cout << "in buildSingle function " << endl;
  //CirMgr* dupOne = ckt0_->duplicate();
  //dupOne->addCircuitCNF(_miterSlover._minisatPtr);
  Var test = _miterSlover._minisatPtr->newVar();
  _miterSlover._minisatPtr->addClause(mkLit(test, false));
  _miterSlover._minisatPtr->addClause(mkLit(test, true));
  // choose common variable

  // using pi first
  addCommonVar(_onSetCir[0]->piList_, _offSetCir[0]->piList_);
  //_miterSlover._minisatPtr->solve();
  //cout << _miterSlover._minisatPtr->solve() << endl;
}

// build circuit solvers
void CirMiter::buildCircuitSolver(bool single) {
  if (single){
    _onSetCir.push_back(ckt0_->duplicate());
    _onSetCir.push_back(ckt1_->duplicate());
    _miterSlover.buildONOFFset(_onSetCir, true, 0); // on set
    //cout _onSetCir
    //cout << "line 157" << endl;
    _offSetCir.push_back(ckt0_->duplicate());
    _offSetCir.push_back(ckt1_->duplicate());
    _miterSlover.buildONOFFset(_offSetCir, false, 0);
  }

}

// optimize circuits
void CirMiter::optimize_full() {
  assert(ckt0_ != NULL && ckt1_ != NULL);
  ckt0_->optimize_full();
  ckt1_->optimize_full();
#ifdef VERBOSE
  ckt0_->reportPoFaninSize();
  ckt1_->reportPoFaninSize();
  ckt0_->reportPiFanoutSize();
  ckt1_->reportPiFanoutSize();
  ckt0_->reportEqPo();
  ckt1_->reportEqPo();
#endif
}



Var CirMiter::buildONOFFset(vector<CirMgr*>& _cirVec, SATMgr& _solMgr, bool ON_OFF, int bit){
  SatSolver* _solver = _solMgr._minisatPtr;

  _cirVec[0]->addCircuitCNF(_solver);
  _cirVec[1]->addCircuitCNF(_solver);

  Var* _on_OR_i_Var = new Var[_cirVec[0]->getPoNum()];
  Var  _on_OR_o_var = _solver->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(_cirVec[0]->getPoNum() + 1);
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)

  for (size_t i=0, n = _cirVec[0]->getPoNum(); i < n; ++i){
    _on_OR_i_Var[i] = _solver->newVar();

    // add output difference gate
    _solver->addXorCNF(_on_OR_i_Var[i], _cirVec[0]->getPoVar(i) , false,
                                        _cirVec[1]->getPoVar(i)   ,false);
    // add (~Input i + Output)
    _solver->addClause(mkLit(_on_OR_i_Var[i], true), mkLit(_on_OR_o_var, false));

    // (~Output += Input 0~n)
    _litVec.push(mkLit(_on_OR_i_Var[i], false));
  }
  _solver->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)

  // bind primary input
  for (size_t i=0, n = _cirVec[0]->getPiNum(); i < n; ++i){
      _solver->addEqCNF(_cirVec[0]->getPiVar(i), _cirVec[1]->getPiVar(i), false);
  }

  _solMgr.resizeMarkVec();

  if (ON_OFF){
    // mark on set
    for (size_t i=0, n = _solver->nClauses(); i < n; ++i){
      _solMgr.markOnSet(i);
    }
    _solver->addClause(mkLit(_cirVec[1]->getWTVar(bit),false), false);
  }
  else{
    // mark off set
    for (size_t i=0, n = _solver->nClauses(); i < n; ++i){
      _solMgr.markOffSet(i);
    }
    _solver->addClause(mkLit(_cirVec[1]->getWTVar(bit),false), true);
  }

  _solver->addClause(mkLit(_on_OR_o_var, false), false); //

  delete []_on_OR_i_Var;
  return _on_OR_o_var;
}




void CirMiter::addCommonVar(GateList& g1, GateList& g2){
  assert(g1.size()==g2.size());

  for (size_t i = 0, n = g1.size(); i < n; ++i){
    _miterSlover._minisatPtr->addEqCNF(g1[i]->getVar(), g2[i]->getVar(), false);
    _miterSlover.mapVar2Name(g1[i]->getVar(), g1[i]->getName());
  }
}



// add same PO match constraint
void CirMiter::addSamePoMatchConstraint() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  for (unsigned i = 0; i < mPo; ++i)
    for (unsigned j=i+1; j < mPo; ++j)
      for (unsigned k=0; k < nPo; ++k) {
        poMatchSolver_.addClause(sameCtrl_,
                                 ~mkLit(poMatchVar_[i][2*k  ]),
                                 ~mkLit(poMatchVar_[j][2*k  ]));
        poMatchSolver_.addClause(sameCtrl_,
                                 ~mkLit(poMatchVar_[i][2*k  ]),
                                 ~mkLit(poMatchVar_[j][2*k+1]));
        poMatchSolver_.addClause(sameCtrl_,
                                 ~mkLit(poMatchVar_[i][2*k+1]),
                                 ~mkLit(poMatchVar_[j][2*k  ]));
        poMatchSolver_.addClause(sameCtrl_,
                                 ~mkLit(poMatchVar_[i][2*k+1]),
                                 ~mkLit(poMatchVar_[j][2*k+1]));
      }
}




// add PO equality constraint
void CirMiter::addEqualConstraint() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  std::vector<unsigned> eq0, eq1;
  std::vector<std::vector<bool> > flg(mPo, std::vector<bool>(nPo, false));
  for (unsigned i = 0; i < mPo; ++i) {
    for (unsigned j = 0; j < nPo; ++j) {
      if (flg[i][j]) continue;
      eq1 = ckt1_->getEqPo(i);
      eq0 = ckt0_->getEqPo(j);
      assert(eq1[0] == 2*i);
      assert(eq0[0] == 2*j);
      // mark flag
      for (unsigned i2=0, sz1=eq1.size(); i2 < sz1; ++i2)
        for (unsigned j2=0, sz0=eq0.size(); j2 < sz0; ++j2)
          flg[eq1[i2]/2][eq0[j2]/2] = true;
      // add constraint
      for (unsigned i2=0, sz1=eq1.size(); i2 < sz1; ++i2) {
        for (unsigned j2=0, sz0=eq0.size(); j2 < sz0; ++j2) {
          if (i2 == 0 && j2 == 0) continue;
          unsigned idx1 = eq1[i2]/2, idx0 = eq0[j2]/2;
          bool phase = (eq1[i2]&1) ^ (eq0[j2]&1);
          if (i2 < j2 || (j2 != sz0-1 && i2 > j2)) {
            poMatchSolver_.addClause(~mkLit(poMatchVar_[idx1][2*idx0  ]));
            poMatchSolver_.addClause(~mkLit(poMatchVar_[idx1][2*idx0+1]));
          } else {
            Lit l1, l2;
            l1 = mkLit(poMatchVar_[i][2*j]);
            l2 = phase? mkLit(poMatchVar_[idx1][2*idx0+1]) :
                        mkLit(poMatchVar_[idx1][2*idx0  ]);
            poMatchSolver_.addClause(~l1,  l2);
            poMatchSolver_.addClause( l1, ~l2);
            l1 = mkLit(poMatchVar_[i][2*j+1]);
            l2 = phase? mkLit(poMatchVar_[idx1][2*idx0  ]) :
                        mkLit(poMatchVar_[idx1][2*idx0+1]);
            poMatchSolver_.addClause(~l1,  l2);
            poMatchSolver_.addClause( l1, ~l2);
          }
        }
      }
    }
  }
}

// disable one PI match
void CirMiter::delPiMatch(unsigned y, unsigned x) {
  piMatchSolver_.addClause(~mkLit(piMatchVar_[y][x]));
  piMatchAble_[y][x] = false;
}

// add PI unused constraint
void CirMiter::addUnusedConstraint() {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  unsigned cnt0 = 0, cnt1 = 0;
  std::vector<bool> used0 = ckt0_->getPiUsed();
  std::vector<bool> used1 = ckt1_->getPiUsed();
  vec<Lit> clause;
  // unused0: should not match
  for (unsigned x=0; x < nPi; ++x) {
    if (!used0[x]) {
      for (unsigned y=0; y < mPi; ++y) {
        delPiMatch(y, 2*x);
        delPiMatch(y, 2*x+1);
      }
    } else {
      cnt0 += 1;
    }
  }
  // unused1: match to const0
  for (unsigned y=0; y < mPi; ++y) {
    if (!used1[y])
      piMatchSolver_.addClause(mkLit(piMatchVar_[y][2*nPi]));
    else
      cnt1 += 1;
  }
  // 1-to-1 and onto
  if (cnt0 == cnt1 && !simple_) {
    constFlg_ = false;
    for (unsigned y=0; y < mPi; ++y) {
      if (used1[y]) {
        delPiMatch(y, 2*nPi);
        delPiMatch(y, 2*nPi+1);
      }
    }
  } else {
    constFlg_ = true;
  }
}


// helper function for addSymmetryConstraint()
void CirMiter::addBlockAllConstraint(const std::vector<unsigned>& grp0,
                                     const std::vector<unsigned>& grp1) {
  for (unsigned i = 0; i < grp0.size(); ++i) {
    for (unsigned j = 0; j < grp1.size(); ++j) {
      delPiMatch(grp1[j]/2, grp0[i]);
      delPiMatch(grp1[j]/2, grp0[i]^1);
    }
  }
}

// helper function for addSymmetryConstraint()
void CirMiter::addSortedConstraint(const std::vector<unsigned>& grp0,
                                   const std::vector<unsigned>& grp1) {
  for (unsigned i0=0; i0 < grp0.size(); ++i0)
    for (unsigned i1=i0+1; i1 < grp0.size(); ++i1)
      for (unsigned j0=0; j0 < grp1.size(); ++j0)
        for (unsigned j1=j0+1; j1 < grp1.size(); ++j1) {
          unsigned x0 = grp0[i0] / 2;
          unsigned x1 = grp0[i1] / 2;
          unsigned y0 = grp1[j0] / 2;
          unsigned y1 = grp1[j1] / 2;
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y1][2*x0]),
                                   ~mkLit(piMatchVar_[y0][2*x1]));
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y1][2*x0]),
                                   ~mkLit(piMatchVar_[y0][2*x1+1]));
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y1][2*x0+1]),
                                   ~mkLit(piMatchVar_[y0][2*x1]));
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y1][2*x0+1]),
                                   ~mkLit(piMatchVar_[y0][2*x1+1]));
        }
}

// add unate constraint
void CirMiter::addUnateConstraint() {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  Unateness u0, u1;
  ckt0_->buildUnateArr();
  ckt1_->buildUnateArr();
  for (unsigned i0=0; i0 < nPi; ++i0) {
    for (unsigned i1=0; i1 < mPi; ++i1) {
      for (unsigned j0=0; j0 < nPo; ++j0) {
        for (unsigned j1=0; j1 < mPo; ++j1) {
          u0 = ckt0_->getUnate(i0, j0);
          u1 = ckt1_->getUnate(i1, j1);
          // block all
          if ((u1.isUnate() && u0.isBinate() && !constFlg_) ||
              (u1.isBinate() && !(u0.isBinate()) && !constFlg_)) {
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0  ]),
                                     ~mkLit(piMatchVar_[i1][2*i0  ]));
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0  ]),
                                     ~mkLit(piMatchVar_[i1][2*i0+1]));
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0+1]),
                                     ~mkLit(piMatchVar_[i1][2*i0  ]));
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0+1]),
                                     ~mkLit(piMatchVar_[i1][2*i0+1]));
          } else if ((u1.isPos() && u0.isPos()) ||
                     (u1.isNeg() && u0.isNeg())) {
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0  ]),
                                     ~mkLit(piMatchVar_[i1][2*i0+1]));
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0+1]),
                                     ~mkLit(piMatchVar_[i1][2*i0  ]));
          } else if ((u1.isPos() && u0.isNeg()) ||
                     (u1.isNeg() && u0.isPos())) {
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0  ]),
                                     ~mkLit(piMatchVar_[i1][2*i0  ]));
            piMatchSolver_.addClause(~mkLit(poMatchVar_[j1][2*j0+1]),
                                     ~mkLit(piMatchVar_[i1][2*i0+1]));
          }
        }
      }
    }
  }
}

// add blocking clause for a failed PI matching
void CirMiter::addBlockingClause(const std::vector<bool>& in0,
                                 const std::vector<bool>& in1,
                                 const std::vector<PUU>& cmpList) {
  unsigned n = in0.size();
  unsigned m = in1.size();
  
     std::cout << " Before generalization: ";
     for (unsigned i = 0; i < n; ++i) std::cout << in0[i];
     std::cout << " ";
     for (unsigned i = 0; i < m; ++i) std::cout << in1[i];
     std::cout << std::endl;
   
  std::vector<bool_3v> in0_3v, in1_3v;
  vec<Lit> clause;
  ckt0_->simulate(in0);
  ckt1_->simulate(in1);
  for (unsigned i = 0, n=cmpList.size(); i < n; ++i) {
    unsigned x = cmpList[i].first/2, y = cmpList[i].second;
    bool inv = cmpList[i].first&1;
    unsigned res0 = ckt0_->getPoSimVal(x);
    unsigned res1 = ckt1_->getPoSimVal(y);
    if (inv) res1 = ~res1;
    if ((res0&1) != (res1&1)) {
      in0_3v = ckt0_->expandCube(in0, x);
      in1_3v = ckt1_->expandCube(in1, y);
      clause.push(~mkLit(poMatchVar_[cmpList[i].second][cmpList[i].first]));
      break;
    }
  }
  assert(clause.size() == 1);
  
     std::cout << " After generalization: ";
     for (unsigned i = 0; i < n; ++i) std::cout << (in0_3v[i] == true_3v? "1" : (in0_3v[i] == false_3v? "0" : "X"));
     std::cout << " ";
     for (unsigned i = 0; i < m; ++i) std::cout << (in1_3v[i] == true_3v? "1" : (in1_3v[i] == false_3v? "0" : "X"));
     std::cout << std::endl;
   
  for (unsigned i = 0; i < m; ++i) {
    if (in1_3v[i] == undef_3v) continue;
    for (unsigned j = 0; j < n; ++j) {
      if (in0_3v[j] == undef_3v) continue;  // HACK, this is not complete
      if (in1_3v[i] != bool_3v(in0[j])) {
        if (piMatchAble_[i][2*j])
          clause.push(mkLit(piMatchVar_[i][2*j  ]));
      } else {
        if (piMatchAble_[i][2*j+1])
          clause.push(mkLit(piMatchVar_[i][2*j+1]));
      }
    }
    if (constFlg_) {
      if (in1_3v[i] == true_3v) {
        if (piMatchAble_[i][2*n])
          clause.push(mkLit(piMatchVar_[i][2*n  ]));
      } else {
        if (piMatchAble_[i][2*n+1])
          clause.push(mkLit(piMatchVar_[i][2*n+1]));
      }
    }
  }
  piMatchSolver_.addClause(clause);
}

// use random simulation to block failed PI matching
void CirMiter::simBlockCtrl(const std::vector<PUU>& cmpList, int nSim) {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  std::vector<bool> in0(nPi), in1(mPi);
  std::vector<unsigned> pattern0(nPi), pattern1(mPi);
  for (int t=0; t < nSim; ++t) {
    for (unsigned i = 0; i < nPi; ++i)
      pattern0[i] = rand();
    for (unsigned i = 0; i < mPi; ++i)
      pattern1[i] = rand();
    ckt0_->simulate(pattern0);
    ckt1_->simulate(pattern1);
    for (unsigned s=0; s < 32; ++s) {
      for (unsigned i = 0, n=cmpList.size(); i < n; ++i) {
        unsigned x = cmpList[i].first/2, y = cmpList[i].second;
        bool inv = cmpList[i].first&1;
        unsigned res0 = ckt0_->getPoSimVal(x);
        unsigned res1 = ckt1_->getPoSimVal(y);
        if (inv) res1 = ~res1;
        if (((res0 >> s) & 1) != ((res1 >> s) & 1)) {
          for (unsigned j = 0; j < nPi; ++j)
            in0[j] = (ckt0_->getPiSimVal(j) >> s) & 1;
          for (unsigned j = 0; j < mPi; ++j)
            in1[j] = (ckt1_->getPiSimVal(j) >> s) & 1;
          addBlockingClause(in0, in1, cmpList);
          break;
        }
      }
    }
  }
}



// write solution to file
void CirMiter::writeSolution() const {
  std::ofstream ofs(outFile_.c_str());
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  // OUTGROUP
  for (unsigned j = 0; j < nPo; ++j) {
    ofs << "OUTGROUP" << std::endl;
    ofs << "1 + " << ckt0_->getPoName(j) << std::endl;
    for (unsigned i = 0; i < mPo; ++i) {
      // positive phase
      if (poMatchSolver_.getVal(poMatchVar_[i][2*j  ]) == 1)
        ofs << "2 + " << ckt1_->getPoName(i) << std::endl;
      // negative phase
      if (poMatchSolver_.getVal(poMatchVar_[i][2*j+1]) == 1)
        ofs << "2 - " << ckt1_->getPoName(i) << std::endl;
    }
    ofs << "END" << std::endl;
  }
  // INGROUP
  for (unsigned j = 0; j < nPi; ++j) {
    ofs << "INGROUP" << std::endl;
    ofs << "1 + " << ckt0_->getPiName(j) << std::endl;
    for (unsigned i = 0; i < mPi; ++i) {
      // positive phase
      if (piMatchSolver_.getVal(piMatchVar_[i][2*j  ]) == 1)
        ofs << "2 + " << ckt1_->getPiName(i) << std::endl;
      // negative phase
      if (piMatchSolver_.getVal(piMatchVar_[i][2*j+1]) == 1)
        ofs << "2 - " << ckt1_->getPiName(i) << std::endl;
    }
    ofs << "END" << std::endl;
  }
  // CONST0GROUP
  ofs << "CONST0GROUP" << std::endl;
  for (unsigned i = 0; i < mPi; ++i) {
    // positive phase
    if (piMatchSolver_.getVal(piMatchVar_[i][2*nPi  ]) == 1)
      ofs << "2 + " << ckt1_->getPiName(i) << std::endl;
    // negative phase
    if (piMatchSolver_.getVal(piMatchVar_[i][2*nPi+1]) == 1)
      ofs << "2 - " << ckt1_->getPiName(i) << std::endl;
  }
  ofs << "END" << std::endl;
  // end file output
  ofs.close();
}

// print current solution
void CirMiter::reportSolution() const {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  for (unsigned i = 0; i < mPi; ++i) {
    for (unsigned j = 0; j < nPi; ++j) {
      if (piMatchSolver_.getVal(piMatchVar_[i][2*j  ]) == 1) {
        std::cout << ckt1_->getPiName(i) << " ->  " << ckt0_->getPiName(j);
        std::cout << std::endl;
      }
      if (piMatchSolver_.getVal(piMatchVar_[i][2*j+1]) == 1) {
        std::cout << ckt1_->getPiName(i) << " -> !" << ckt0_->getPiName(j);
        std::cout << std::endl;
      }
    }
    if (piMatchSolver_.getVal(piMatchVar_[i][2*nPi  ]) == 1) {
      std::cout << ckt1_->getPiName(i) << " ->  CONST0";
      std::cout << std::endl;
    }
    if (piMatchSolver_.getVal(piMatchVar_[i][2*nPi+1]) == 1) {
      std::cout << ckt1_->getPiName(i) << " -> !CONST0";
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
}

*/

