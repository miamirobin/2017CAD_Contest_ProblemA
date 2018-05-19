/****************************************************************************
  FileName     [ cirMiter.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Circuit miter class ]
  Author       [ music960633 ]
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

// read Verilog
void CirMiter::readVerilog(const std::string& filename0,
                           const std::string& filename1) {
  init();
  ckt0_ = new CirMgr;
  ckt1_ = new CirMgr;
  ckt0_->readVerilog(filename0);
  ckt1_->readVerilog(filename1);
  // PO controls
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  poMatchVar_.resize(mPo);
  for (unsigned i = 0; i < mPo; ++i)
    poMatchVar_[i].resize(2*nPo);
  // PI controls
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  piMatchVar_.resize(mPi);
  for (unsigned i = 0; i < mPi; ++i)
    piMatchVar_[i].resize(2*nPi + 2);
  piMatchAble_.resize(mPi);
  for (unsigned i = 0; i < mPi; ++i)
    piMatchAble_[i].resize(2*nPi + 2);
}

// optimize circuits
void CirMiter::optimize_full() {
  
  assert(ckt0_ != NULL && ckt1_ != NULL);
  
  ckt0_->optimize_full();
  ckt1_->optimize_full();
  ckt0_->reportAllPo();
  ckt1_->reportAllPo();
#ifdef VERBOSE
  ckt0_->reportPoFaninSize();
  ckt1_->reportPoFaninSize();
  ckt0_->reportPiFanoutSize();
  ckt1_->reportPiFanoutSize();
  ckt0_->reportEqPo();
  ckt1_->reportEqPo();
  
  
#endif
}

// initialize solvers
void CirMiter::initSolver() {
  poMatchSolver_.init();
  piMatchSolver_.init();
  circuitSolver_.init();
  circuitSolver_.setRndFreq(0.0015);
  buildPoMatchSolver();
  buildPiMatchSolver();
  buildCircuitSolver();
  bindControl();
  poMatchSolver_.simplify();
  piMatchSolver_.simplify();
  circuitSolver_.simplify();
}

// build PO match solver
void CirMiter::buildPoMatchSolver() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  vec<Lit> clause;
  // create PO control variables
  std::vector<std::pair<unsigned, PUU> > poVec;
  for (unsigned i = 0; i < mPo; ++i)
    for (unsigned j = 0; j < 2*nPo; ++j)
      poVec.push_back(std::make_pair(
          ckt0_->getPoFaninSize(j/2) + ckt1_->getPoFaninSize(i),
          std::make_pair(j, i)
          ));
  std::sort(poVec.begin(), poVec.end());
  for (unsigned i = 0, n = poVec.size(); i < n; ++i) {
    unsigned x = poVec[i].second.first;
    unsigned y = poVec[i].second.second;
    poMatchVar_[y][x] = poMatchSolver_.newVar();
    // create corresponding vars in other solvers
    piMatchSolver_.newVar();
    circuitSolver_.newVar();
  }
  // map to at most one PO
  for (unsigned i = 0; i < mPo; ++i)
    for (unsigned j = 0; j < 2*nPo; ++j)
      for (unsigned k=j+1; k < 2*nPo; ++k)
        poMatchSolver_.addClause(~mkLit(poMatchVar_[i][j]),
                                 ~mkLit(poMatchVar_[i][k]));
  // at least one match
  for (unsigned i = 0; i < mPo; ++i)
    for (unsigned j = 0; j < 2*nPo; ++j)
      clause.push(mkLit(poMatchVar_[i][j]));
  poMatchSolver_.addClause(clause);
  // create controls
  sameCtrl_ = mkLit(poMatchSolver_.newVar());
  trivialCtrl_ = mkLit(poMatchSolver_.newVar());
  // add other constraints
  addSamePoMatchConstraint();
  addTrivialPoConstraint();
  addEqualConstraint();
}

// build PI match solver
void CirMiter::buildPiMatchSolver() {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  // create PI control variables
  for (unsigned i = 0; i < mPi; ++i) {
    for (unsigned j = 0; j < 2*nPi+2; ++j) {
      piMatchVar_[i][j] = piMatchSolver_.newVar();
      piMatchAble_[i][j] = true;
      // create corresponding vars in other solvers
      circuitSolver_.newVar();
    }
  }
  // add other constraints
  addUnusedConstraint();
  if (!simple_) {
    addSymmetryConstraint();
    addUnateConstraint();
  }
  // set 1-hot constraint
  for (unsigned i = 0; i < mPi; ++i) {
    vec<Lit> clause;
    // map to at least one PI:
    // (a0 + b0 + a1 + b1 + ... + an + bn)
    for (unsigned j = 0; j < 2*nPi+2; ++j)
      if (piMatchAble_[i][j])
        clause.push(mkLit(piMatchVar_[i][j]));
    piMatchSolver_.addClause(clause);
    // map to at most one PI:
    // (~a0 + ~b0) & (~a0 + ~a1) & ... & (~an + ~bn)
    for (unsigned j = 0; j < 2*nPi+2; ++j)
      for (unsigned k=j+1; k < 2*nPi+2; ++k)
        if (piMatchAble_[i][j] && piMatchAble_[i][k])
          piMatchSolver_.addClause(~mkLit(piMatchVar_[i][j]),
                                   ~mkLit(piMatchVar_[i][k]));
  }
}

// build circuit solvers
void CirMiter::buildCircuitSolver() {
  ckt0_->addCircuitCNF(&circuitSolver_);
  ckt1_->addCircuitCNF(&circuitSolver_);
}

// bind PO and PI controls
void CirMiter::bindControl() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  // bind PO controls
  vec<Lit> cexClause;
  for (unsigned i = 0; i < mPo; ++i) {
    for (unsigned j = 0; j < 2*nPo; ++j) {
      Lit poCtrl = mkLit(poMatchVar_[i][j]);
      Lit out0 = mkLit(ckt0_->getPoVar(j>>1));
      Lit out1 = mkLit(ckt1_->getPoVar(i));
      Lit eq   = mkLit(circuitSolver_.newVar());
      Lit cex  = mkLit(circuitSolver_.newVar());
      if (j&1) out0 = ~out0;
      // (out0 == out1) -> eq
      circuitSolver_.addClause( out0,  out1, eq);
      circuitSolver_.addClause(~out0, ~out1, eq);
      // (~poCtrl | eq) -> ~cex
      circuitSolver_.addClause( poCtrl, ~cex);
      circuitSolver_.addClause(~eq    , ~cex);
      // add cex to cexClause
      cexClause.push(cex);
    }
  }
  circuitSolver_.addClause(cexClause);
  // bind PI controls
  for (unsigned i = 0; i < mPi; ++i) {
    for (unsigned j = 0; j < 2*nPi+2; ++j) {
      Lit piCtrl = mkLit(piMatchVar_[i][j]);
      Lit in0 = mkLit(j >= 2*nPi ? ckt0_->getConstVar() :
                                   ckt0_->getPiVar(j>>1));
      Lit in1 = mkLit(ckt1_->getPiVar(i));
      if (j&1) in0 = ~in0;
      // piCtrl -> (in0 == in1)
      circuitSolver_.addClause(~piCtrl,  in0, ~in1);
      circuitSolver_.addClause(~piCtrl, ~in0,  in1);
    }
  }
}

// main NP3 solve function
void CirMiter::solveNP3() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  simple_ = true;
  // do second time if constFlg_ is false at the first time
  bestRating_ = 0;
  do {
    initSolver();
    simpleCounter_ = 0;
    for (unsigned effort = 1; effort <= 1; effort <<= 1) {
      std::cout << "========================" << std::endl;
      std::cout << "Effort set to " << std::setw(10) << effort << std::endl;
      std::cout << "========================" << std::endl;
      abortCtrl_ = mkLit(poMatchSolver_.newVar());
      solvePoMatch(effort);
      poMatchSolver_.addClause(~abortCtrl_);
      if (bestRating_ >= maxRating_) break;
    }
    if (simple_) {
      simple_ = false;
      setMaxRating(nPo, mPo);
    } else {
      break;
    }
  } while (bestRating_ < maxRating_);
  std::cout << "=========" << std::endl;
  std::cout << "Complete!" << std::endl;
  std::cout << "=========" << std::endl;
}

// solve PO matching
void CirMiter::solvePoMatch(int effort) {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  vec<Lit> assump, clause;
  int result;
  std::vector<PUU> cmpList;
  assump.push(abortCtrl_);
  assump.push(~sameCtrl_);
  assump.push(~trivialCtrl_);
  while (bestRating_ < maxRating_) {
    simpleCounter_ += 1;
    if (simple_ && simpleCounter_ > 100) break;
    if (poMatchSolver_.solve(assump)) {
      // get assumptions
      vec<Lit> piMatchAssump;
      cmpList.clear();
      for (unsigned i = 0; i < mPo; ++i) {
        for (unsigned j = 0; j < 2*nPo; ++j) {
          if (poMatchSolver_.getVal(poMatchVar_[i][j]) == 1) {
            piMatchAssump.push(mkLit(poMatchVar_[i][j]));
            cmpList.push_back(std::make_pair(j, i));
          } else {
            piMatchAssump.push(~mkLit(poMatchVar_[i][j]));
          }
        }
      }
      // solve
#ifdef VERBOSE
      reportComparePoint(cmpList);
#endif
      result = solvePiMatch(piMatchAssump, cmpList, effort);
      if (result == 1) {
        // EQ
        updateRating(cmpList);
        clause.clear();
        for (unsigned i = 0; i < mPo; ++i)
          for (unsigned j = 0; j < 2*nPo; ++j)
            if (poMatchSolver_.getVal(poMatchVar_[i][j]) == 0) {
              clause.push(mkLit(poMatchVar_[i][j]));
            } else {
              int idx = 0, n = assump.size();
              while(idx < n && assump[idx] != mkLit(poMatchVar_[i][j]))
                idx += 1;
              if (idx == n)
                assump.push(mkLit(poMatchVar_[i][j]));
            }
        poMatchSolver_.addClause(clause);
      } else if (result == -1) {
        // Abort
        clause.clear();
        clause.push(~abortCtrl_);
        for (unsigned i = 0; i < mPo; ++i)
          for (unsigned j = 0; j < 2*nPo; ++j) {
            if (poMatchSolver_.getVal(poMatchVar_[i][j]) == 1)
              clause.push(~mkLit(poMatchVar_[i][j]));
          }
        poMatchSolver_.addClause(clause);
      } else {
        // NEQ, result = 0
        clause.clear();
        for (unsigned i = 0; i < mPo; ++i)
          for (unsigned j = 0; j < 2*nPo; ++j)
            if (poMatchSolver_.getVal(poMatchVar_[i][j]) == 1)
              clause.push(~mkLit(poMatchVar_[i][j]));
        poMatchSolver_.addClause(clause);
      }
    } else if (sign(assump[2]) &&
               rating(cmpList) + trivialPoCnt_ * 12 > bestRating_) {
      // trivialCtrl off and has potential -> turn on trivialCtrl
      assump.push(mkLit(Var(0)));
      assump[2] = trivialCtrl_;
    } else if (assump.size() > 3) {
      // search reach end, stack not empty -> remove assump
      if (assump.last() == mkLit(Var(0))) {
        assump.pop();
        assump[2] = ~trivialCtrl_;
      }
      assump.pop();
    } else if (sign(assump[1])) {
      // search reach end, stack is empty -> turn on sameCtrl
      assump[1] = sameCtrl_;
    } else {
      break;
    }
  }
}

// solve PI matching
int CirMiter::solvePiMatch(const vec<Lit>& piMatchAssump,
                           const std::vector<PUU>& cmpList,
                           unsigned effort) {
  unsigned nPi = ckt0_->getPiNum();
  unsigned mPi = ckt1_->getPiNum();
  unsigned fail = 0;
  // maximum fail limit: magic number
  const unsigned param = (ckt0_->getGateNum() + ckt1_->getGateNum()) * effort;
  const double rndFreq = (param > 2000 ? 0.007 : 0.001);
  const unsigned maxFail = (simple_ ? 100 : param * 4);
  const unsigned numSim = param / 50;
  piMatchSolver_.setRndFreq(rndFreq);
#ifdef VERBOSE
  std::cout << "[" << "max fail: " << maxFail << " , "
            << "best rating: " << bestRating_ << " ]" << std::endl;
  std::cout << std::endl;
#endif
  // simulation
  simBlockCtrl(cmpList, numSim);
  // solve PI matching
  while (piMatchSolver_.solve(piMatchAssump)) {
#ifdef VERBOSE
    std::cout << "trying #" << fail << " ...\r" << std::flush;
#endif
    fail += 1;
    if (fail > maxFail) {
#ifdef VERBOSE
      std::cout << std::endl;
      std::cout << "Fail reach limit" << std::endl;
      std::cout << std::endl;
#endif
      return -1;
    }
    // solve SAT
    vec<Lit> circuitAssump;
    for (unsigned i = 0, sz=piMatchAssump.size(); i < sz; ++i)
      circuitAssump.push(piMatchAssump[i]);
    for (unsigned i = 0; i < mPi; ++i)
      for (unsigned j = 0; j < 2*nPi+2; ++j)
        if (piMatchSolver_.getVal(piMatchVar_[i][j]) == 1)
          circuitAssump.push(mkLit(piMatchVar_[i][j]));
    if (circuitSolver_.solve(circuitAssump)) {
      // add blocking clause
      std::vector<bool> in0(nPi), in1(mPi);
      for (unsigned i = 0; i < nPi; ++i)
        in0[i] = circuitSolver_.getVal(ckt0_->getPiVar(i));
      for (unsigned i = 0; i < mPi; ++i)
        in1[i] = circuitSolver_.getVal(ckt1_->getPiVar(i));
      addBlockingClause(in0, in1, cmpList);
    } else {
#ifdef VERBOSE
      std::cout << std::endl;
      std::cout << "Solution found!" << std::endl;
      std::cout << "Fail = " << fail - 1 << std::endl;
      std::cout << std::endl;
#endif
      // reportSolution();
      return 1;
    }
  }
#ifdef VERBOSE
  std::cout << std::endl;
  std::cout << "No solution" << std::endl;
  std::cout << std::endl;
#endif
  return 0;
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

// add PO trvial constraint
void CirMiter::addTrivialPoConstraint() {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  unsigned cnt = 0;
  unsigned bound;
  for (unsigned i = 0; i < nPo; ++i)
    if (ckt0_->getPoFaninPiSize(i) <= 1)
      cnt += 1;
  for (unsigned i = 0; i < mPo; ++i)
    if (ckt1_->getPoFaninPiSize(i) <= 1)
      cnt += 1;
  bound = (cnt > 5 ? 8 : 3);
  addTrivialPoConstraintWithBound(bound);
}

void CirMiter::addTrivialPoConstraintWithBound(unsigned bound) {
  unsigned nPo = ckt0_->getPoNum();
  unsigned mPo = ckt1_->getPoNum();
  unsigned cnt0 = 0, cnt1 = 0;
  std::vector<unsigned> faninSize0(nPo);
  std::vector<unsigned> faninSize1(mPo);
  vec<Lit> clause;
  for (unsigned i = 0; i < nPo; ++i)
    if ((faninSize0[i] = ckt0_->getPoFaninPiSize(i)) <= bound)
      cnt0 += 1;
  for (unsigned i = 0; i < mPo; ++i)
    if ((faninSize1[i] = ckt1_->getPoFaninPiSize(i)) <= bound)
      cnt1 += 1;
  trivialPoCnt_ = std::max(cnt0, cnt1);
  if (simple_) {
    for (unsigned i = 0; i < mPo; ++i) {
      clause.clear();
      for (unsigned j = 0; j < 2*nPo; ++j) {
        if (faninSize0[j/2] > bound)
          poMatchSolver_.addClause(~mkLit(poMatchVar_[i][j]));
        else
          clause.push(mkLit(poMatchVar_[i][j]));
      }
      if (faninSize1[i] <= bound)
        poMatchSolver_.addClause(clause);
    }
    setMaxRating(cnt0, mPo);
  } else if (trivialPoCnt_ <= 6) {
    for (unsigned i = 0; i < mPo; ++i)
      for (unsigned j = 0; j < 2*nPo; ++j)
        if (faninSize0[j/2] <= bound || faninSize1[i] <= bound)
          poMatchSolver_.addClause(trivialCtrl_, ~mkLit(poMatchVar_[i][j]));
  } else if (trivialPoCnt_ > 10) {
    trivialPoCnt_ = std::max(nPo - cnt0, mPo - cnt1);
    for (unsigned i = 0; i < mPo; ++i)
      for (unsigned j = 0; j < 2*nPo; ++j)
        if (faninSize0[j/2] > bound)
          poMatchSolver_.addClause(~mkLit(poMatchVar_[i][j]));
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

// add PI symmetry constraint
void CirMiter::addSymmetryConstraint() {
  int x, y, xSym, ySym, inv;
  std::vector<std::vector<unsigned> > symGrps0 = ckt0_->getSymmetricPiGrps();
  std::vector<std::vector<unsigned> > symGrps1 = ckt1_->getSymmetricPiGrps();
  // sym match non-sym
  for (unsigned i=1; i < symGrps0.size(); ++i) {
    if (symGrps0.size() == symGrps1.size())
      addBlockAllConstraint(symGrps0[i], symGrps1[0]);
    else
      addSortedConstraint(symGrps0[i], symGrps1[0]);
  }
  for (unsigned i=1; i < symGrps1.size(); ++i)
    addSortedConstraint(symGrps0[0], symGrps1[i]);
  // sym match sym -> 1 to 1
  for (unsigned i=1; i < symGrps0.size(); ++i) {
    for (unsigned j=1; j < symGrps1.size(); ++j) {
      std::vector<unsigned>& grp0 = symGrps0[i];
      std::vector<unsigned>& grp1 = symGrps1[j];
      if (grp0.size() > grp1.size()) {
        addBlockAllConstraint(grp0, grp1);
        continue;
      }
      for (unsigned k=0; k < grp0.size(); ++k) {
        x    = grp0[0] / 2;
        y    = grp1[0] / 2;
        xSym = grp0[k] / 2;
        ySym = grp1[k] / 2;
        inv = (grp0[k] & 1) ^ (grp1[k] & 1);
        if (k > 0) {
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y][2*x  ]),
                                    mkLit(piMatchVar_[ySym][2*xSym + inv]));
          piMatchSolver_.addClause( mkLit(piMatchVar_[y][2*x  ]),
                                   ~mkLit(piMatchVar_[ySym][2*xSym + inv]));
          piMatchSolver_.addClause(~mkLit(piMatchVar_[y][2*x+1]),
                                    mkLit(piMatchVar_[ySym][2*xSym + 1 - inv]));
          piMatchSolver_.addClause( mkLit(piMatchVar_[y][2*x+1]),
                                   ~mkLit(piMatchVar_[ySym][2*xSym + 1 - inv]));
        }
        for (unsigned l=0; l < grp0.size(); ++l) {
          if (l == k) continue;
          delPiMatch(ySym, grp0[l]);
          delPiMatch(ySym, grp0[l]^1);
        }
      }
      for (unsigned k=grp0.size(); k < grp1.size(); ++k)
        for (unsigned l=0; l < grp0.size(); ++l) {
          delPiMatch(grp1[k]/2, grp0[l]);
          delPiMatch(grp1[k]/2, grp0[l]^1);
        }
    }
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
  /*
     std::cout << " Before generalization: ";
     for (unsigned i = 0; i < n; ++i) std::cout << in0[i];
     std::cout << " ";
     for (unsigned i = 0; i < m; ++i) std::cout << in1[i];
     std::cout << std::endl;
   */
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
  /*
     std::cout << " After generalization: ";
     for (unsigned i = 0; i < n; ++i) std::cout << (in0_3v[i] == true_3v? "1" : (in0_3v[i] == false_3v? "0" : "X"));
     std::cout << " ";
     for (unsigned i = 0; i < m; ++i) std::cout << (in1_3v[i] == true_3v? "1" : (in1_3v[i] == false_3v? "0" : "X"));
     std::cout << std::endl;
   */
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

// print matched PO
void CirMiter::reportComparePoint(const std::vector<PUU>& cmpList) const {
  std::cout << "Solving:" << std::endl;
  for (unsigned i = 0, n=cmpList.size(); i < n; ++i) {
    unsigned x = cmpList[i].first / 2;
    unsigned y = cmpList[i].second;
    bool inv = cmpList[i].first & 1;
    std::cout << ckt1_->getPoName(y) << " -> " << (inv? "!" : "")
              << ckt0_->getPoName(x) << std::endl;
  }
  std::cout << std::endl;
}

// initialize rating
void CirMiter::setMaxRating(unsigned x, unsigned y) {
  bestRating_ = 0;
  if (x >= y)
    maxRating_ = y * 12;
  else
    maxRating_ = x * 12 + (y - x);
}

// rating for a PO matching
unsigned CirMiter::rating(const std::vector<PUU>& sol) const {
  unsigned rate = 0;
  bool flg;
  for (unsigned i = 0, n=sol.size(); i < n; ++i) {
    flg = true;
    for (unsigned j = 0; j < i; ++j) {
      if (sol[i].first/2 == sol[j].first/2) {
        flg = false;
        break;
      }
    }
    if (flg)
      rate += 12;
    else
      rate += 1;
  }
  return rate;
}

// update rating by a solution
void CirMiter::updateRating(const std::vector<PUU>& solution) {
  unsigned newRating = rating(solution);
  if (newRating > bestRating_) {
    bestRating_ = newRating;
    std::cout << "Update rating: " << newRating << std::endl;
    std::cout << "Writing solution ..." << std::endl;
    std::cout << std::endl;
    writeSolution();
  }
}
