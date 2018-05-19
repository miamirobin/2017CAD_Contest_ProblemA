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

typedef std::pair<unsigned, unsigned> PUU;

/**************************************
    Class CirMiter definition
**************************************/
class CirMiter{
 public:
  CirMiter();
  ~CirMiter();

  // initialize
  void     init();

  // functions for building circuit
  void     readVerilog(const std::string& filename1,
                       const std::string& filename2);
  void     optimize_full();

  // functions for initializing all solvers
  void     initSolver();
  void     buildPoMatchSolver();
  void     buildPiMatchSolver();
  void     buildCircuitSolver();
  void     bindControl();

  // functions for solving NPNP equivalence
  void     solveNP3();
  void     solvePoMatch(int effort);
  int      solvePiMatch(const vec<Lit>& piMatchAssump,
                        const std::vector<PUU>& cmpList,
                        unsigned effort);

  // functions for adding CNF constraint
  void     addSamePoMatchConstraint();
  void     addTrivialPoConstraint();
  void     addTrivialPoConstraintWithBound(unsigned bound);
  void     addEqualConstraint();
  void     delPiMatch(unsigned y, unsigned x);
  void     addUnusedConstraint();
  void     addSymmetryConstraint();
  void     addUnateConstraint();
  void     addBlockAllConstraint(const std::vector<unsigned>& grp0,
                                 const std::vector<unsigned>& grp1);
  void     addSortedConstraint(const std::vector<unsigned>& grp0,
                               const std::vector<unsigned>& grp1);
  void     addBlockingClause(const std::vector<bool>& in0,
                             const std::vector<bool>& in1,
                             const std::vector<PUU>& cmpList);
  void     simBlockCtrl(const std::vector<PUU>& cmpList, int nSim);

  // functions for writing and reporting
  void     setOutputFile(const std::string& s) { outFile_ = s; }
  void     writeSolution() const;
  void     reportSolution() const;
  void     reportComparePoint(const std::vector<PUU>& cmpList) const;

  // functions for CAD Contest rating
  void     setMaxRating(unsigned x, unsigned y);
  unsigned rating(const std::vector<PUU>& solution) const;
  void     updateRating(const std::vector<PUU>& solution);

 private:
  CirMgr*                          ckt0_;            // golden circuit
  CirMgr*                          ckt1_;            // revised circuit
  SatSolver                        poMatchSolver_;   // PO matching solver
  SatSolver                        piMatchSolver_;   // PI matching solver
  SatSolver                        circuitSolver_;   // circuit cex solver
  std::vector<std::vector<Var> >   poMatchVar_;      // PO matching controls
  std::vector<std::vector<Var> >   piMatchVar_;      // PI matching controls
  std::vector<std::vector<bool> >  piMatchAble_;     // PI matching able flag
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
};

#endif  // CIR_CIRMITER_H_
