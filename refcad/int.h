/* Copyright NTUEE 2009. All Rights Reserved. */
/* =====================================================================================
 *       Filename:  int.h
 *    Description:  Interpolation Main Class to Solve ECO problem
 *        Created:  08/03/09 00:41:12 CST
 *         Author:  Gary Wu (NTUEE), researchgary@gmail.com
 *        Company:  NTUEE
 * =====================================================================================*/

#ifndef  INT_H
#define  INT_H

#ifndef AIG_H
#include "aig.h"
#endif

#include "intbuilder.h"
#include "stdint.h"

#include <map>
#include <sstream>
#include <string>

/**************************************************************************************
Class:  IntEco				Time:08/03/09 00:43:59 CST
Description:  Control main process 
 **************************************************************************************/

enum SET_TYPE{
   ONSET  = 0,
   OFFSET
};

enum OPERATION_MODE{
   FANOUT_POS = 0,
   ONE_PO,
   BOTH
};

enum ITP_SOLVER{
   ITP_WSAT = 0,
   ITP_MINISAT
};

struct SolveParameters{
   bool isUseNameMapping;
   bool isSimplified;
   bool onlyItp;
   unsigned simulationTimes;
   double   simulationSlope;
   OPERATION_MODE mode; 
   bool dominatorDetect;
   double limRatio;
   unsigned fix_po_num;
};

class PatchInfo;

class IntEco
{
   friend class IntMgr;
   public:
   IntEco ():_solverUse(ITP_WSAT){
      _aigNetDup = new AigNet();
      _onSet     = new AigNet();
      _offSet    = new AigNet();
      _commonNet = new AigNet(); 
      _onoff     = new AigNet(); 
      _wsatMgr   = new WSatMgr(); 
      _wsatInt = new WSatInt();
      _judgeCand.resize(3);
      _mode = FANOUT_POS;
      unsigned levels[] = { 3,0 };
      _levels.resize(2);
      for ( unsigned i = 0 ; i < _levels.size() ; ++i )
         _levels[i] = levels[i];
   }
   ~IntEco(){ 
      delete _aigNetDup;
      delete _onSet;
      delete _offSet;
      delete _commonNet;
      delete _onoff;
      delete _wsatMgr;
      delete _wsatInt;
   }

   void              readNet( AigNet* );

   bool              itp_solve(const SolveParameters&);

   // build all the template networks for on and off set
   void              itp_build();            ///< build all needed template networks
   void              itp_combineOnOff(bool); ///< combine onSet net and offSet net
   void              itp_on( AigNet& );      ///< build onSet net
   void              itp_off( AigNet& );     ///< build offSet net
   void              itp_setType();          ///< setup all base type and var type for all gates and variables
   void              itp_iterative();        ///< iteratively solve ECO
   int               itp_getItp(const AigNode&,const vector<AigNode>&, const vector<AigNode>&); ///< build itp net corresponding to that candidate
   bool              itp_checkPO();          ///< return all po is fixed or not
   void              itp_lockPO();           ///< equivalence checking before itp-based method
   void              itp_showInfo(bool);     ///< simply show patches info
   bool              itp_judge();            ///< check the chosen node can be a cand or not
   void              itp_sort(unsigned);     ///< get all infos of all nodes and sort poInfo
   void              itp_common( AigNode , unsigned ); ///< set fanin of n as global vars within l levels
   void              itp_replace(vector<AigNode>&,unsigned);    ///< relace the candidate node by itp function
   void              itp_selNode( AigNode& , vector<AigNode>&, unsigned, AigNode&, const SolveParameters& );        ///< according to poSorted to choose candidate node
   void              itp_recover(bool,AigNode&, vector<AigNode>&, vector<AigNode>& ); ///< recover to the old-cand state from cand-free state
   void              itp_cntItpPIO( AigNet&, AigNet&, AigNode&, int);   ///< connect patches to the original net
   vector<AigNode>   itp_poLimit( AigNet&, bool );             ///< all pos are in constraints
   void              itp_commonRec( AigNode, unsigned l, vector<AigNode>&, vector<bool>& );  ///< set fanin of n as global vars within l levels
   void              itp_sortRec( AigNode, vector<bool>&, unsigned&, AigNode, unsigned );    ///< recusively sorting poInfo
   AigNode           itp_findNode( const AigNode&, const vector<AigNode>&, bool );           ///< find a node from a vector by name or by id
   AigNode           itp_poDynamicLimit( vector<AigNode>&, bool);           ///< choose some pos in constraints
   void              itp_assNode( AigNode& , const vector<AigNode>&, vector<AigNode>&, vector<AigNode>& ); ///< set net for the chosen node
   void              itp_reset();
   bool              itp_judgeSim(unsigned&);
   //bool              itp_sim( AigNode, vector<bool>&);
   uint64_t          itp_sim( AigNode, vector<bool>&, vector<uint64_t>&);
   //bool          itp_sim( AigNode, vector<bool>&, vector<bool>&);
   void              itp_setSolver(ITP_SOLVER);
   vector<unsigned>  itp_backupId( AigNet& );
   void              itp_restoreId( AigNet&, const vector<unsigned>&);
   void              itp_getNameMap();
   void              itp_nameMapping();
   void              itp_extractNet(AigNet&);
   void              itp_lowPriorityNodes();
   void              itp_dominator();           ///< find dominators for all nodes
   void              itp_wblif(AigNet&,string); ///< blif dumping
   void              itp_wblif_rec(AigNet&,AigNode&,ofstream&, vector<bool>&); ///< blif dumping
   void              itp_ntkClean(const AigNet& innet, AigNet& outnet,unsigned nNodes = 0 );
   void              itp_ntkCleanRec(AigNet&, AigNode,vector<bool>&);
   void              itp_toSolver( Solver& s, IntParam& param, vec<Lit>&);
   void              itp_toSolver_fast( Solver& s, IntParam& param, vec<Lit>& assumps );
   void              itp_toSolverRec( AigNode&, Solver&, IntParam&, bool, vector<bool>& );
   bool              cec(AigNet&,AigNet&);

   void              itp_patchinfo_backup(const AigNet&, const AigNode&, const vector<AigNode>&, const vector<AigNode>&);  ///< store new-generated patch(larger than limit)
   bool              itp_patchinfo_restore(AigNode&,vector<AigNode>&, vector<AigNode>&); 
                     ///< choose the smallest obtained too-large patch
   void              itp_patchinfo_clean();   ///< clean all stored too-large patch

   vector<bool>      itp_backup_visited(AigNet&);
   void              itp_restore_visited(AigNet&,const vector<bool>&);
   void              itp_set_cone_visited(AigNet&, AigNode& , bool);
   void              itp_set_cone_visited_rec(AigNode&, bool, vector<bool>&);

   bool              itp_final_fix();  ///< directly use golden network
   void              itp_if_error(unsigned,unsigned);

   private:
   SolveParameters      _sp;        ///< Process parameters
   AigNet*              _aigNet;    ///< input eco circuit
   AigNet*              _aigNetDup; ///< duplicate input eco circuit
   AigNet*              _onSet;     ///< itp onset
   AigNet*              _offSet;    ///< itp offset
   AigNet*              _commonNet; ///< itp common variable set
   AigNet*              _onoff;     ///< itp combined on, off, and common
   AigNet               _patch;     ///< combine all the patches generated by interpolation
   vector<AigNet>       _patchSet;  ///< collect all patches in this vector
   vector<unsigned>     _patchSize; ///< collect all off-line optimized patch sizein this vector

   AigNode              _const1A;   ///< const one node in onSet
   AigNode              _const1B;   ///< const one node in offSet
   vector<AigNode>      _poLimit;   ///< store PO-constrainters
   vector<AigNode>      _poSet;     ///< store original all POs[size=8*_nPO]
   vector<AigNode>              _commonNodes;      ///< store the inputs of patches each time
   vector<AigNode>              _commonNodesDup;   ///< the other three dupplicated inputs of patches
   vector<vector<AigNode> >     _faninNodeBackup;  ///< store the fanins of chosen cand(used for recover)
   vector<vector<AigNode> >     _faninPOBackup;    ///< store the PO-constrainters for chosen cand(used for recover)
   vector<vector<unsigned> >     _dominator;    ///< store all dominators
   vector<unsigned>     _levels;    ///< the levels for searching inputs of patches
   vector<PatchInfo>             _backupPatches;   ///< back-up patches

   long _sim_total;
   long _sim_success;
   long _sim_times;

   WSatMgr*             _wsatMgr;   ///< WSat solving manager
   WSatInt*             _wsatInt;   ///< WSat interpolation manager

   map<WSatBitVar,VAR_TYPE>    _mapVar;
   vector<BASE_TYPE>           _mapBase;
   int                         _ithCand;
   vector<AigNode>             _changedNode;
   OPERATION_MODE              _mode;
   ITP_SOLVER                  _solverUse; // 0: WSat, 1:MiniSAT
   map<size_t,bool>            _nameMap;

   //TODO
   typedef pair<AigNode,vector<AigNode> >  PoPair;
   typedef pair<AigNode,vector<AigNode> >  NodePair;
   vector<PoPair>    _poSorted;
   map<size_t,vector<AigNode> >  _nodeInfo;
   vector<NodePair>  _nodeSorted;
   map<size_t,bool>  _poDone;
   unsigned          _nPO;
   vector<AigNode>   _judgeCand; ///< used to judge cand nodes: size = 3

   AigNode aigXor ( const AigNode& , const AigNode& ) const;
   AigNode aigXnor( const AigNode& , const AigNode& ) const;
   AigNode crConst1( AigNode );
   AigNet  dupNet(AigNet&,string);
   AigNet  dupRevisedNetNoPI( AigNet& );
   void    buildNet( AigNet&, AigNode&, bool );
   void    buildRec( AigNet&, AigNode&, map<size_t,bool>&, bool );
   void    itp_vecIntersect(const vector<unsigned>&, vector<unsigned>&);
   unsigned sum1( map<size_t,bool>& );
   size_t  normalize(AigNode) const ;
   void    itp_draw(AigNet&,string);

}; /* classEnd_IntEco */


/*
 * Required information for a patch to replace
 * All nodes are just reference, not create new nodes
 */
class PatchInfo{
   friend class IntEco;
   PatchInfo(  const AigNet& net, const AigNode& cand, const vector<AigNode>& cmnNodes, 
               const vector<AigNode>& cmnNodesDup, const vector<vector<AigNode> >& fnNodeBup,
               const vector<vector<AigNode> >& fnPOBup, const vector<AigNode>& poAffected,
               const vector<AigNode>& poIgr
            ){
      _patch = net;
      _cand = cand;
      _commonNodes = cmnNodes;
      _commonNodesDup = cmnNodesDup;
      _faninNodeBackup = fnNodeBup;
      _faninPOBackup = fnPOBup;
      _poAffected = poAffected;
      _poIgr = poIgr;
   }
   private:
      AigNet                    _patch;
      AigNode                   _cand;
      vector<AigNode>           _commonNodes;
      vector<AigNode>           _commonNodesDup;
      vector<AigNode>           _poAffected;
      vector<AigNode>           _poIgr;
      vector<vector<AigNode> >  _faninNodeBackup;
      vector<vector<AigNode> >  _faninPOBackup;
};

#endif   /* #ifndef INT_H  */
