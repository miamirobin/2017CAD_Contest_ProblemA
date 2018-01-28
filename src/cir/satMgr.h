/****************************************************************************
  FileName     [ satMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ sam031023 ]
****************************************************************************/

#ifndef SAT_MGR_H
#define SAT_MGR_H

#include <cassert>
#include <vector>
#include <map>
#include "cir/cirMgr.h"
#include "minisat/reader.h"
#include "minisat/sat.h"



using namespace std;

enum VAR_GROUP {
  LOCAL_ON,
  LOCAL_OFF,
  COMMON,
  NONE
};

class CirMiter;
class CirMgr;

class SATMgr{

friend class CirMiter;

	public:
		SATMgr():_minisatPtr(NULL){}
		~SATMgr(){}
                unsigned t_num=0;
                unsigned aig_num=0;
		void reset();
                void resetsolver();

                void reset2();   //minisat 2.20
                void resetsolver2();   //minisat 2.20

		void readVerilog(const string& filename1, const string& filename2, const string& filename3 );
                void  BindPi(GateList& g1, GateList& g2);
		void build();
		void addCommonVar(GateList& g1, GateList& g2);
                void addComControl(CirMgr* c1, CirMgr* c2);
                void optimize_patch(vector<string>& _patchName,int i ); //optimize the target node
		// utility function
		int getNumClauses() const{ return _minisatPtr->nClauses();}
		bool checkEq(vector<CirMgr*>& _cirVec);
                void writepatch(const string& filenamef,const string& filename,const string& filename1);
                void buildControlVar1();
                void buildControlVar2();
                static bool myComparison1(const pair<vector<unsigned>,string> &a,const pair<vector<unsigned>,string> &b);
                static bool myComparison2(const pair<vector<unsigned>,string> &a,const pair<vector<unsigned>,string> &b);

	private:
                void buildforcheckEq();
		void buildSingle();
                void buildSingle2();
                void buildSingleSS();
                void buildMulti();
                void buildMultiple();
                void buildMultiple2(int wt);
                void buildSingle_2(); //minisat2.20
                void buildFunctionDependency();
                void buildforMultiple(vector<CirMgr*>& _cirVec,vector<bool> &rvset);
		void buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);
                void buildONOFFset_2(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);  //minisat 2.20
                void buildONOFFset2(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset);
                void buildONOFFset2forITP(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset);
                void buildONOFFsetforFunctionDependency(vector<CirMgr*>& _cirVec, bool ON_OFF);
                void buildONOFFsetforItp(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);
		void buildONOFFsetSS(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);
                void mapVar2Name(const Var& var, const string& name );
                
                bool cofactorReduction();
		void markOnSet(const ClauseId cid);
		void markOffSet(const ClauseId cid);
                void resetRM();
                bool SolvenRM(vector<CirMgr*>& _cirVec );
		void resizeMarkVec();
                void SolveRCset(vector<CirMgr*>& _cirVec,vector<bool> &rvset);
		// helper function to get proof info.
		void getItp();
		string buildItp(const string& proofName);
		void retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<int>& usedClasuse);
		void retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;
                
                // cut findding functions, implement in cutFind.cpp
                void sol_sin_plus();
                void sol_sin_minus();
                void sol_sin_zero();
                void sol_sin_one(); // try to find one net to solve problem
                void sol_sin_two(); // take out nets with big weight first
                void sol_sin_three();
                void sol_sin_four();
                void sol_sim_five();
                void sol_mul_one();
                void sol_mul_two();
                void sol_mul_zero();

		CirMgr*	ckt0_;            // golden circuit
		CirMgr* ckt1_;            // revised circuit
                vector<CirMgr*> _checkCir;  // circuit after revised
		vector<CirMgr*> _onSetCir;  // on set circuit
		vector<CirMgr*> _offSetCir; // off set circuit
                vector<CirMgr*> rm;
                vector<vector<CirMgr*>> rcset;
                vector<vector<CirMgr*>> _onSetVec;
                vector<vector<CirMgr*>> _offSetVec;
                vector<bool> rvset;
                vector<vector<bool>>             rvsetVec;
                SatSolver*                               solver0 ;
                vector <vector<CirMgr*> > veccir; // store all the onset and offset circuit
		bool _isSingle;
                
		SatSolver*				_minisatPtr;
		map<Var, string> 	_var2Name;
		vector<bool> 			_isClauseOn;
                map<string, unsigned> 					_weightMap;		// gate name to weight map
		map<string, Var>                _gate2ConVar;	// mapping gate name to controlling var
		vector <pair<vector<unsigned>, string>>	_weightOrder; // weight to gate name, from small to big
		Var*			        _OrderedVar;	// Vars to controll two gate is common or not
                vector <string>           commonVar;
                vector <string>           Cut;
		vector<string>		_patchNameVec;
		//
                int _nVar;
		vector<bool>        _isClaOnDup;
		vector<VAR_GROUP>   _varGroup;
                int             BestCost=0;
                int             Cost=0;
                vector <unsigned> WandL;
                int _itpNewMark = 0;
                bool     NoTime=false;
};

#endif //SAT_MGR_H
