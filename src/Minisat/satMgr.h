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
#include "Minisat/satt.h"
#include "cir/cirMgr.h"

using namespace std;

class CirMiter;
class CirMgr;

class SATMgr{

friend class CirMiter;

	public:
		SATMgr():_minisatPtr(NULL){}
		~SATMgr(){}

		void 	reset();

		void 	markOnSet(const int clauseId);
		void 	markOffSet(const int clauseId);

		void 	resizeMarkVec();

		void	buildONOFFset(vector<CirMgr*>& _cirVec, bool ON_OFF, int bit);
		

		// utility function
		int 	getNumClauses() const{ return _minisatPtr->nClauses();}


	private:
		SatSolver*				_minisatPtr;
		map<Var, string> 	_var2Name;
		vector<bool> 			_isClauseOn; 
};

#endif //SAT_MGR_H
