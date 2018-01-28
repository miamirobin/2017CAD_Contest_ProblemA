/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ minisat ]
  Synopsis     [ functions for using minisat ]
  Author       [ sam031023 ]
****************************************************************************/

#ifndef MINISAT_SAT_H_
#define MINISAT_SAT_H_

#include "SolverV.h"

#include <iostream>
#include <cassert>

/*
using Minisat::Solver;
using Minisat::Var;
using Minisat::Lit;
using Minisat::mkLit;
using Minisat::vec;
using Minisat::lbool;
using Minisat::l_True;
using Minisat::l_False;
using Minisat::l_Undef;
*/
class SATMgr;

/**************************************
  Class SatSolver
 **************************************/
class SatSolver {

  friend class SATMgr;

 public:
  SatSolver(): solver_(NULL) {}
  ~SatSolver() {
    if(solver_ != NULL) delete solver_;
  }

  // initialize solver
  void init(bool _needProof = true) {
    if(solver_ != NULL) delete solver_;
    solver_ = new SolverV();
    //solver_->newVar();
    if (_needProof)
			solver_->proof = new Proof();
  }

  int nVars() const { return solver_->nVars(); }
  int nClauses() const { return solver_->nRootCla();}

  //void setRndFreq(double freq) { solver_->random_var_freq = freq; }

  void addClause(Lit lit0) { solver_->addUnit(lit0); }
  void addClause(Lit lit0, Lit lit1) { solver_->addBinary(lit0, lit1); }
  void addClause(Lit lit0, Lit lit1, Lit lit2) {
    solver_->addTernary(lit0, lit1, lit2);
  }
  void addClause(const vec<Lit>& clause) { solver_->addClause(clause); }

  const bool simplify();

  // functions for adding clauses
  Var newVar() { return solver_->newVar(); }
  void addUnitCNF(Var var, bool inv);
  void addEqCNF(Var var0, Var var1, bool inv);
  void addAndCNF(Var out, Var in0, bool inv0, Var in1, bool inv1);
  void addXorCNF(Var out, Var in0, bool inv0, Var in1, bool inv1);
  void addxorCNF(Var var0, Var var1, bool inv);
  // functions for solving
  const bool solve() { solver_->solve(); return solver_->okay(); }
  bool solve(Var var, bool val) {
    vec<Lit> tmp;
    tmp.push(mkLit(var, !val));//mkLit(var, !val)
    return solver_->solve(tmp);
  }
  bool solve(const vec<Lit>& assump) { return solver_->solve(assump); }
  bool simplifyDB() {return solver_->simplifyDB();}
  /*
  lbool solveLimited(int64_t budget) {
    vec<Lit> assump;
    return solveLimited(assump, budget);
  }
  lbool solveLimited(Var var, bool val, int64_t budget) {
    vec<Lit> assump;
    assump.push(mkLit(var, !val));
    return solveLimited(assump, budget);
  }
  lbool solveLimited(const vec<Lit>& assump, int64_t budget) {
    solver_->setConfBudget(budget);
    return solver_->solveLimited(assump);
  }*/
  unsigned getVal(Var var) const {
    if(solver_->model[var] == l_True ) return 1;
    assert(solver_->model[var] == l_False);
    return 0;
  }

 private:
  SolverV* solver_;
};

#endif  // MINISAT_SAT_H_
