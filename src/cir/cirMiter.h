/****************************************************************************
  FileName     [ cirMiter.h ]
  PackageName  [ cir ]
  Synopsis     [ Circuit miter class ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CIR_CIRMITER_H_
#define CIR_CIRMITER_H_

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "cir/cirMgr.h"
#include "minisat/sat.h"
#include "cir/satMgr.h"

using namespace std;

extern bool _globalVerbosityFlg;

typedef std::pair<unsigned, unsigned> PUU;

/**************************************
    Class CirMiter definition
**************************************/
class CirMiter{
 public:
  CirMiter();
  ~CirMiter();
/*
  // initialize
  void     init();

	bool 		 reportSingle(){ return _isSingle; }

  void     optimize_full();

  // functions for cad 2017
  void     initSolver();
	void     buildList();
	void     readVerilog(const std::string& filename1, const std::string& filename2);
        void     readWeight(const std::string& filename);
  void     buildSingle();
  void     buildCircuitSolver(bool single = true);
 
	bool 		 solve() { return  _miterSlover._minisatPtr->solve(); }

  // functions for adding CNF constraint
  void     addSamePoMatchConstraint();
  void     addEqualConstraint();
  void     delPiMatch(unsigned y, unsigned x);
  void     addUnusedConstraint();
  void     addUnateConstraint();
  void     addBlockAllConstraint(const std::vector<unsigned>& grp0,
                                 const std::vector<unsigned>& grp1);
  void     addSortedConstraint(const std::vector<unsigned>& grp0,
                               const std::vector<unsigned>& grp1);
  void     addBlockingClause(const std::vector<bool>& in0,
                             const std::vector<bool>& in1,
                             const std::vector<PUU>& cmpList);
  void     simBlockCtrl(const std::vector<PUU>& cmpList, int nSim);

  void     addCommonVar(GateList& g1, GateList& g2);

  // functions for writing and reporting
  
  void     setOutputFile(const std::string& s) { outFile_ = s; }
  void     writeSolution() const;
  void     reportSolution() const;

  // function for build miter


 private:
  CirMgr*                          ckt0_;            // golden circuit
  CirMgr*                          ckt1_;            // revised circuit

  vector<CirMgr*>                   _onSetCir;       // on set circuit
  vector<CirMgr*>                   _offSetCir;      // off set circuit

  SATMgr                           _miterSlover;     // miter solver

  bool                             _isSingle;

  SatSolver                        poMatchSolver_;   // PO matching solver
  SatSolver                        piMatchSolver_;   // PI matching solver
  SatSolver                        circuitSolver_;   // circuit cex solver



  std::vector<std::vector<Var> >   poMatchVar_;      // PO matching controls
  std::vector<std::vector<Var> >   piMatchVar_;      // PI matching controls
  std::vector<std::vector<bool> >  piMatchAble_;     // PI matching able flag

  //vector<Var>                      _commonVar;       // common variables
  //Var*                             _Id2Var;

  bool                             constFlg_;        // can match to constant
  bool                             simple_;          // simple mode
  unsigned                         simpleCounter_;   // counter for cmpList
  unsigned                         trivialPoCnt_;    // # of trivial POs
  Lit                              abortCtrl_;       // abort clause control
  Lit                              sameCtrl_;        // match same PO control
  Lit                              trivialCtrl_;     // trivial PO control
  unsigned                         maxRating_;       // maximum possible rating
  unsigned                         bestRating_;      // current best rating
  std::string                      outFile_;         // output file name
*/
};

#endif  // CIR_CIRMITER_H_
