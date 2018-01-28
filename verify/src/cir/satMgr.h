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
//#include "minisat/reader.h"
#include "minisat/satt.h"



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

		void readVerilog(const string& filename0, const string& filename1, const string& filename2, const string& filename3);
                
		void build();
		void addCommonVar(GateList& g1, GateList& g2);
                void optimize_patch(vector<string>& _patchName,int i ); //optimize the target node
		// utility function
		int getNumClauses() const{ return _minisatPtr->nClauses();}
		bool checkEq(vector<CirMgr*>& _cirVec);
                void writepatch(const string& filenamef,const string& filename,const string& filename1);
	private:
                void buildforcheckEq();
		void buildSingle();
                void buildSingle_2(); //minisat2.20
                void buildFunctionDependency();
                void buildMultiple();void buildMultiple2();
		void buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);
                void buildONOFFset_2(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);  //minisat 2.20
                void buildONOFFset2(vector<CirMgr*>& _cirVec, bool ON_OFF, vector<bool> &rvset);
                void buildONOFFsetforFunctionDependency(vector<CirMgr*>& _cirVec, bool ON_OFF);
		void mapVar2Name(const Var& var, const string& name );

		//void markOnSet(const ClauseId cid);
		//void markOffSet(const ClauseId cid);
                void markOnSet(const int clauseId);
                void markOffSet(const int clauseId);
		bool cofactorReduction();
		
                void resetRM();
                bool SolvenRM(vector<CirMgr*>& _cirVec );
		void resizeMarkVec();
                void SolveRCset(vector<CirMgr*>& _cirVec,vector<bool> &rvset);

		// helper function to get proof info.
		void getItp();
		string buildItp(const string& proofName);
		//void retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<int>& usedClasuse);
		//void retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;


		CirMgr*	ckt0_;            // golden circuit
		CirMgr* ckt1_;            // revised circuit
                vector<CirMgr*> _checkCir;  // circuit after revised
		vector<CirMgr*> _onSetCir;  // on set circuit
		vector<CirMgr*> _offSetCir; // off set circuit
                vector <vector<CirMgr*> > veccir; // store all the onset and offset circuit
		bool _isSingle;
                 vector<CirMgr*> rm;
                vector<vector<CirMgr*>> rcset;
                vector<bool> rvset;
                vector<vector<bool>>             rvsetVec;
                SatSolver*                               solver0 ;
              
		SatSolver*				_minisatPtr;
		map<Var, string> 	_var2Name;
		vector<bool> 			_isClauseOn;
                
		vector<string>		_patchNameVec;
		//
                int _nVar;
		vector<bool>        _isClaOnDup;
		vector<VAR_GROUP>   _varGroup;
};

#endif //SAT_MGR_H
