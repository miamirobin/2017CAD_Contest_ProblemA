/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Circuit manager class ]
  Author       [ sam031023 ]
****************************************************************************/

#ifndef CIR_CIRMGR_H_
#define CIR_CIRMGR_H_

#include <string>
#include <vector>
#include <map>

#include "cir/cirGate.h"
#include "minisat/satt.h"
//#include "minisat/satMgr.h"


class CirMiter;
class SATMgr;

using namespace std;

/**************************************
    Class CirMgr definition
**************************************/
// circuit manager
class CirMgr {

  friend class CirMiter;
  friend class SATMgr;

 public:

  explicit CirMgr(const std::string& name = "");
  ~CirMgr();

  // functions for initializing
  void init();
  void setName(const std::string& name) { name_ = name; }

  // function for building data structures
  void buildAigList();
  void buildTopoList();
  void buildt0TopoList();
  void buildUnateArr();
  void buildSelectPoTopoList();
  void selectPo();void selectPoS();
  void selectPi();
  // functions for copy circuit
  CirMgr* duplicate() const;

  // functions for getting values
  std::string getName()             const { return name_; }
  unsigned    getGateNum()          const { return gateList_.size(); }
  unsigned    getPiNum()            const { return piList_.size(); }
  unsigned    getPoNum()            const { return poList_.size(); }
  unsigned    getAigNum()           const { return aigList_.size(); }
  unsigned    getWtNum()            const { return tList_.size();}
  unsigned    getTopoNum()          const { return topoList_.size();}
  unsigned    getSelPiNum()         const { return piNeedList.size(); }

  CirGate*    getGate(int id)       const { return gateList_[id]; }
  CirGate*    getConstGate()        const { return const0_; }
  CirGate*    getPiGate(int id)     const { return piList_[id]; }
  CirGate*    getPoGate(int id)     const { return poList_[id]; }
  CirGate*    getAIGate(int id)     const { return aigList_[id]; }
  CirGate*    getWtGate(int id)     const { return tList_[id]; }
  CirGate*    getGateByName(string& name)    const { return name2GateMap_.find(name)->second; }

  std::string getPiName(int id)     const { return piList_[id]->getName(); }
  std::string getPoName(int id)     const { return poList_[id]->getName(); }
  std::string getAigName(int id)    const { return aigList_[id]->getName(); }
  std::string getWtName(int id)    const { return tList_[id]->getName(); }

  unsigned    getPiSimVal(int id)   const { return piList_[id]->getSimVal(); }
  unsigned    getPoSimVal(int id)   const { return poList_[id]->getSimVal(); }
  bool_3v     getPiSimVal3v(int id) const { return piList_[id]->getSimVal3v(); }
  bool_3v     getPoSimVal3v(int id) const { return poList_[id]->getSimVal3v(); }
  Unateness   getUnate(int i, int j)  const { return unateArr_[i][j]; }
  Var         getConstVar()         const { return const0_->getVar(); }
  Var         getTopoVar(int id)      const { return topoList_[id]->getVar(); }
  Var         getPiVar(int id)      const { return piList_[id]->getVar(); }
  Var         getPoVar(int id)      const { return poList_[id]->getVar(); }
  Var         getAigVar(int id)     const { return aigList_[id]->getVar(); }
  Var         getSelPiVar(int id)   const {return  piNeedList[id]->getVar();}
  Var         getWtVar(int id)      const { assert(tList_.size() > id); return tList_[id]->getVar(); }
  GateList&	getAigList() { return aigList_; }
  GateList&	getPiList() { return piList_; }
  GateList&	getPoList() { return poList_; }
  GateList&	getTopoList() { return topoList_; }
  GateList&     getSelPiList() {return piNeedList;}
  int       getwtSize() { return tList_.size(); }

  int                   getPiIdx(const CirGate* gate) const;
  int                   getPoIdx(const CirGate* gate) const;
  bool                  getGateUsed(CirGate* gate) const;
  bool                  getPiUsed(int id) const;
  std::vector<bool>     getPiUsed() const;
  unsigned              getGateFaninSize(CirGate* gate) const;
  unsigned              getGateFaninPiSize(CirGate* gate) const;
  unsigned              getPoFaninSize(int idx) const;
  unsigned              getPoFaninPiSize(int idx) const;
  std::vector<unsigned> getPoFaninSizeVec() const;
  unsigned              getGateFanoutSize(CirGate* gate) const;
  unsigned              getPiFanoutSize(int idx) const;
  std::vector<unsigned> getPiFanoutSizeVec() const;
  std::vector<unsigned> getEqPo(unsigned id) const;

  // functions for finding symmetric groups
  //std::vector<std::vector<unsigned> > getSymmetricPiGrps();
  void simPiSymmetry      (const std::vector<bool>& P,
                           std::vector<std::vector<bool> >* posSym,
                           std::vector<std::vector<bool> >* negSym);
  void buildSymmetryMiter (SatSolver* solver,
                           std::vector<Var>* piVar0,
                           std::vector<Var>* piVar1,
                           std::vector<Var>* ctrl);



  // [Implemented in cirReport.cpp]
  // functions for reporting
  void report() const;
  void reportNetlist() const;
  void reportAllPo() const;
  void reportAigList() const;
  void reportTopoList() const;
  void reportPoFaninSize() const;
  void reportPiFanoutSize() const;
  void reportFecGrpList() const;
  void reportUnateArr() const;
  void reportEqPo() const;

  // [Implemented in cirConstruct.cpp]
  // functions for constructing circuit
  void      addToMap    (const std::string& name, CirGate* gate);
  void      delFromMap  (const std::string& name);
  CirGate*  addConstGate(const std::string& name = "");
  CirGate*  addPiGate   (const std::string& name = "");
  CirGate*  addPoGate   (const std::string& name = "");
  CirGate*  addAIGate   (const std::string& name = "");
  CirGate*  addWtGate   (const string& name);
  void      delAIGate   (unsigned idx);
  void      connect1inputGate(const std::string& type,
                              CirGate* out, CirGate* in0);
  void      connect2inputGate(const std::string& type,
                              CirGate* out, CirGate* in0, CirGate* in1);
  void      connectPo   (CirGate* gate, CirGate* in0);
  void      connectAIG  (CirGate* gate,
                         CirGate* in0, bool inv0,
                         CirGate* in1, bool inv1);
  void      connectnot (CirGate* out,  CirGate* in0, bool inv0 );
  void      connectBuf  (CirGate* out, CirGate* in0);
  void      connectNot  (CirGate* out, CirGate* in0);
  void      connectAnd  (CirGate* out, CirGate* in0, CirGate* in1);
  void      connectNand (CirGate* out, CirGate* in0, CirGate* in1);
  void      connectOr   (CirGate* out, CirGate* in0, CirGate* in1);
  void      connectNor  (CirGate* out, CirGate* in0, CirGate* in1);
  void      connectXor  (CirGate* out, CirGate* in0, CirGate* in1);
  void      connectXnor (CirGate* out, CirGate* in0, CirGate* in1);
  void      replaceGate (CirGate* orig, CirGateV repl);

  // [Implemented in cirParse.cpp]
  // functions for reading circuits
  bool readout(const std::string& filename);
  bool readparse(const std::string& filename);
  bool readAig(const std::string& filename);
  bool readVerilog(const std::string& filename);
  void readWeight(const std::string& filename);
  bool readVerilogandWriteAig(const std::string& filename,const std::string& filename1);
  bool removeComment(const std::string& inFilename,
                     const std::string& outFilename) const;
  bool parse(const std::string& filename);
  void parseWeight(const std::string& filename);
  bool parseOut(const std::string& );
  bool parsePatch(const std::string&  );
  // [Implemented in cirWrite.cpp]
  // functions for writing circuits
  void writeAig(const std::string& filename);
  void writePatch(const std::string& filenamef, const std::string& filename,const std::string& filename1);
  // [Implemented in cirOpt.cpp]
  // functions for optimizing
  void     optimize_full();
  unsigned sweep();
  unsigned optimize();
  unsigned strash();
  void     changeBase(CirGate*);
  void     changeBasetopo(CirGate* poGate,SatSolver *solver);
  // [Implemented in cirSim.cpp]
  // functions for simulation
  void                  simulate();
  void                  simulate(const std::vector<unsigned>& pattern);
  void                  simulate(const std::vector<bool>& pattern);
  void                  simulate3v(const std::vector<bool_3v>& pattern);
  std::vector<bool_3v>  expandCube(const std::vector<bool>& pattern, int id);
  void                  delFecGrpList();
  void                  initFecGrpList();
  bool                  updateFecGrpList();
  bool                  updateFecGrp(FecGrp* oldFecGrp,
                                     FecGrpList* newFecGrpList);
  void                  collectFecGrp(const std::map<unsigned, FecGrp*>& mp,
                                      FecGrpList* fecGrpList);

  // functions for fraig
  void fraig(int effort = -1);
  void initSolver(SatSolver* solver);
  void addCircuitCNF(SatSolver* solver);
  void addPoCNF(SatSolver* solver) ;
  lbool checkConst0(SatSolver* solver, CirGateV gateV, int effort) const;
  lbool checkEquivalence(SatSolver* solver,
                         CirGateV gateV0, CirGateV gateV1, int effort) const;
  void updateBySAT(SatSolver* solver);

 private:
  // private member functions
  void dfs_backward(CirGate* gate) const;
  void dfs_forward(CirGate* gate) const;
  void buildTopoList_rec(CirGate* gate);
  void buildt0TopoList_rec(CirGate* gate);
  void selectPorec(CirGate* t0);void selectPorecS(CirGate* t0);
  void buildSelectPoTopoList_rec(CirGate* gate);
  void selectPiRec(CirGate* _g);
   
  std::string                            name_;
  unsigned                               maxId_;
  CirGate*                               const0_;
  GateList                               gateList_;
  GateList                               piList_, poList_, aigList_;
  GateList                               tList_;
  GateList                               topoList_;
  GateList                               oritopoList_;
  GateList                               oripoList_, oriaigList_,t0topolist;
  GateList                               baselist;
  GateList                               poneedlist, SelectPotopoList_,piNeedList;                                          
  std::map<std::string, CirGate*>        name2GateMap_;
  std::vector<unsigned>                  poNeedNumlist;
  FecGrpList                             fecGrpList_;
  std::vector<std::vector<Unateness> >   unateArr_;
};

#endif  // CIR_CIRMGR_H_
