/* Copyright NTUEE 2009. All Rights Reserved. */
/* =====================================================================================
 *       Filename:  int.cpp
 *    Description:  Define functions in IntEco class
 *        Created:  08/03/09 01:04:24 CST
 *         Author:  Gary Wu (NTUEE), researchgary@gmail.com
 * ===================================================================================== */

#include "int.h"
#include "aigMain.h"
#include "prsBar.h"
#include "util.h"

#include <set>
#include <algorithm>
#include <sstream>
#include <math.h>

using namespace std;
#define ITP_SHOW
//#define OFFLINE_OPT 
#define TIMING
map<size_t,bool> poDone;

//extern AigNet optimizeByABC(AigNet);

#ifdef TIMING
myTimer time_sim("simulation");
myTimer time_dom("dominator");
myTimer time_sel("candidate selection");
myTimer time_itp("interpolation");
myTimer time_opt("optimization");
myTimer time_cec("equivalence checking");
#endif


/* ---TOP FUNCTION: itpSolve-------------------------*//**
Build nets & translate into WSat solver and solve
----------------------------------------------------\n*/
bool
IntEco::itp_solve( const SolveParameters& sp ){
   itp_setSolver(ITP_MINISAT);
   _sp = sp;
   _mode = _sp.mode;
   itp_build();
   /* 1. Global var must have the same name at on and off set. \n
    * 2. 3 POs: on -> off -> global constraints \n */
   if ( _sp.isUseNameMapping )
      itp_nameMapping();
   itp_sort(1);
   itp_iterative();
   return 1;
}


/* ---  FUNCTION: itp_iterative-------------------------*//**

   Iteratively solve: Main Algorithm

 1. Use "itp_selNode" to select a node from _nodeSorted
 2. Use "itp_setType" to set up types of A and B
 3. Use "itp_assNode" to assign const value to the selected node
 4. Use "itp_common"  to build common-variable net to _onoff and set global types
 5. Use "itp_recover" to recover the net which is modified by (3) and (4) to original net
 6. Use "itp_getItp"  to get an interpolant if UNSAT
 7. Use "itp_replace" to put the generated interpolant into _onSet and _offSet
 8. Use "itp_judge"   to judge the selected node is candidate or not(complete)
 9. Use "itp_judgeSim"to judge the selected node is candidate or not by simulation(not completely judge)
 10.Use "itp_dominator" to compute dominators of each nodes(fast prune non-candidates)
 11.Use "itp_lockPO"  to lock the fixed POs by SAT
 12.Use "itp_final_fix" to fix unfixed POs at last

   _mode:
      ONE_PO      : fix at least one PO of a candidate in its fanout cone
      FANOUT_POS  : fix at all POs of a candidate in its fanout cone
      BOTH        : Mix the above two mode

   _poDone:
      All elements are 1: ECO Done
      Else              : Iterative ECO process

   this_po_giveup : if true, fix this PO at itp_final_fix

\n*///-----------------------------------------------
void
IntEco::itp_iterative(){
   for ( unsigned i = 0 ; i < _nPO ; ++i )
      _poDone[normalize(_poSet[i+_nPO])] = 0;
   // Lock func-eq POs before starting
   itp_lockPO();
   if ( _sp.dominatorDetect){
      itp_dominator();
   }
   if ( itp_checkPO() ){
      cout << "The two networks are equivalent..." << endl;
      return;
   }
   unsigned nDiffs = _nPO-sum1(_poDone);
   #ifdef ITP_SHOW
   PrsBarEco prs;
   if ( !_sp.isSimplified ){
      prs.updateState("Loading...");
      prs.initial(_aigNet->getNumNode(),_aigNet->getNumPO()/2,sum1(_poDone) );
      prs.printFrame(0,true);
      prs.updateState("Processing...");
   }
   #endif
   unsigned num_tested_po = 0;
   do{ // fix all POs
      AigNode cand;
      vector<AigNode> poAffected, candDup, poIgr;
      int status = 1;
      itp_reset();
      unsigned ith;
      num_tested_po++;
      bool this_po_giveup=false;
      for ( ith = 0 ; ith < _levels.size() ; ++ith ){

         if ( _mode == BOTH && ith == _levels.size()-2 ){ 
            _mode = ONE_PO;
            ith = 0;
         }
         vector<bool> checked(_onSet->getNumNode(),0);
         AigNode po;
         do{   // fix selected POs
            poAffected.clear();
            itp_selNode(cand,poAffected,ith, po, _sp);
            if ( itp_findNode(cand,_changedNode,0)() )
               continue;
            candDup.clear();
            itp_setType();
            #ifdef ITP_SHOW
            if ( !_sp.isSimplified )
               prs.updateCand(cand.getName(),_levels[ith]);
            #endif

            /* A: Candidate Selection */
            if ( checked[cand.getId()]==0 ){
               // Check only once at ith=0, all PIs are common vars
               if ( _sp.dominatorDetect && ith==0 ){
                  itp_common(cand,0);
                  itp_assNode(cand,poAffected,poIgr,candDup);
               }else{
                  itp_common(cand,_levels[ith]);
                  itp_assNode(cand,poAffected,poIgr,candDup);
               }
               if ( !itp_judgeSim(_sp.simulationTimes) && !itp_judge() )
                  checked[cand.getId()]=1;
            }
            if ( !checked[cand.getId()] ){
               itp_recover( true/*recover all backup*/, cand, poIgr, candDup );
               if ( _sp.dominatorDetect){
                  for ( unsigned i = 0 ; i < _dominator[cand.getId()].size() ; ++i )
                     checked[_dominator[cand.getId()][i]] = 0; 
               }
               continue;
            }
            
            /* B: Obtain Interpolant */
            if (_sp.dominatorDetect){
               itp_recover( true/*recover all backup*/, cand, poIgr, candDup );
               itp_common(cand,_levels[ith]);
               itp_assNode(cand,poAffected,poIgr,candDup);
            }
            status = itp_getItp(cand,poAffected,poIgr);
            /* C: Choose smallest patch from backup or give-up this PO */
            if ( status==-1 ){
               itp_recover(1,cand,poIgr, candDup);
               this_po_giveup = itp_patchinfo_restore(cand,poAffected,poIgr);
               itp_patchinfo_clean();
               if ( this_po_giveup )
                  break;
               candDup[0] = cand;
               candDup[1] = _onoff->_nodeList[cand.getId()+_onSet->getNumNode()];
               itp_common(cand,_levels[ith]);
               itp_assNode(cand,poAffected,poIgr,candDup);
               itp_recover(0,cand,poIgr, candDup );
               status = 0;
               break;
            }
            if (status==1){
               itp_recover(1,cand,poIgr, candDup);
               continue;
            }
            itp_recover(0,cand,poIgr, candDup );
         }while(_ithCand>0 && status == 1 ); // end of selecting nodes

         if ( status == 0 ) break;
         else _ithCand = _nodeSorted.size();
      } // end of levels
      if ( !this_po_giveup ){
         itp_replace(candDup,_levels[ith]);
         _changedNode.push_back(cand);
         unsigned before = sum1(_poDone);
         itp_lockPO();
         itp_if_error(before,sum1(_poDone));
      }
      #ifdef ITP_SHOW
      if ( !_sp.isSimplified )
         prs.updatePatchInfo(_patchSet.back().getNumNode()-_patchSet.back().getNumPI(),
                             _patchSet.size(), sum1(_poDone));
      #endif
      if ( itp_checkPO() == 1 )
         break;
   }while( num_tested_po!=nDiffs ); // end of ECO process
   #ifdef ITP_SHOW
   if ( !_sp.isSimplified ){
      prs.updateState("Completed");
      prs.printFrame(0,true);
   }
   #endif
   bool byFinal = itp_final_fix();
   if ( _sp.isSimplified )
      itp_showInfo(byFinal);
}

/*
 * Just directly use golden network as patch
 * a. Do this for unfixed POs
 * b. Only count node numbers but not really fix the old network
 * Return true: fix by this way
 * Return false: net already fixed
 */
bool
IntEco::itp_final_fix(){
   vector<pair<AigNode,AigNode> > po_pairs;
   for ( unsigned i = 0 ; i < _nPO ; ++i ){
      AigNode old_po = _poSet[i+_nPO];
      AigNode golden_po = _poSet[i];
      if ( _poDone[normalize(old_po)] == 0 ){
         po_pairs.push_back(make_pair(old_po,golden_po));
      }
   }
   AigNet diff_po_net, patch;
   if ( po_pairs.size()==0 )
      return false;
   for ( unsigned i = 0 ; i < po_pairs.size() ; ++i )
      diff_po_net._poList.push_back(po_pairs[i].second);
   itp_ntkClean(diff_po_net,patch,_onoff->getNumNode());
   _patchSet.push_back(patch);
   _patchSize.push_back(patch.getNumNode()-patch.getNumPI());
   return true;
}

/*
 * One candidate must at-least one PO
 * Print error message about the wrong patch
 */
void
IntEco::itp_if_error( unsigned before, unsigned after ){
   if ( before < after )
      return;
   cout << before << " " << after << endl;
   cout << "patch NO." << _patchSet.size() << endl;
   cout << "_ithCand = " << _ithCand << endl;
   cout << "#node of patch = " << _patchSet.back().getNumNode() - _patchSet.back().getNumPI() << endl;
   cout << "#pi of patch = " << _patchSet.back().getNumPI() << endl;
   itp_draw(_patchSet.back(),"patch.png");
   assert(0);
}

/*
 * Briefly show the results of this ECO process
 */
void
IntEco::itp_showInfo(bool byFinal){
   cout << endl;
   cout << "--------------* Patch Info *---------------" << endl;
   cout << "  Patch is generated..." << endl;
   cout << "  Number of sub-patches: " << _patchSet.size() << endl;
   cout << "  ";
   unsigned nNodes = 0;
   for ( unsigned i = 0 ; i < _patchSet.size() ; ++i ){
      nNodes += _patchSet[i].getNumNode() - _patchSet[i].getNumPI();
      if ( i == _patchSet.size()-1 )
         cout << _patchSet[i].getNumNode()- _patchSet[i].getNumPI() << " = ";
      else
         cout << _patchSet[i].getNumNode()- _patchSet[i].getNumPI() << " + ";
   }
   cout << nNodes << endl << endl;
   cout << "  Simulated nodes:   " << _sim_times << endl;
   cout << "  Success nodes:     " << _sim_success << " " << (double)_sim_success/(double)_sim_times << endl;
   cout << "  Simulated pattern: " << _sim_total << endl;
   if ( byFinal )
      cout << "  P.S. Some POs are fixed by using golden network." << endl;
}

/*
 * used to sum up _poDone for fixed POs
 */
unsigned
IntEco::sum1(  map<size_t,bool>& a ){
   unsigned sum = 0;
   for ( map<size_t,bool>::iterator it = a.begin() ; it!=a.end() ; ++it )
      if ( it->second )
         ++sum;
   return sum;
}

/*
 * Recover those nodes modified at itp_assNode and itp_common
 * isSat: 
 *    => true  : recover the selected node and POs
 *    => false : recover only POs (the candidate will be replace by interpolant)
 */
void
IntEco::itp_recover( bool isSat, AigNode& cand, vector<AigNode>& poIgr, vector<AigNode>& candDup ){
   // recover candidate node fanins
   if ( isSat ){
      for ( unsigned i = 0 ; i < 2 ; ++i ){
         candDup[i].setFanin( 0, _faninNodeBackup[i][0] );
         candDup[i].setFanin( 1, _faninNodeBackup[i][1] );
      }
      _onoff->_nodeList.resize(_onSet->getNumNode()*2+2);
      _onoff->_poList[0] = _onSet->_poList[0];
      _onoff->_poList[1] = _offSet->_poList[0];
      candDup.clear();
   }
   // recover itp_poLimit fanins
   for ( unsigned i = 0 ; i < poIgr.size() ; ++i ){
      AigNode n = poIgr[i];
      n.setFanin( 0, _faninPOBackup[i][0] );
      n.setFanin( 1, _faninPOBackup[i][1] );
   }
   for ( unsigned i = 0 ; i < _commonNodes.size() ; ++i )
      _mapVar[WSatBitVar(_commonNodes[i].getId(),0)] = VAR_A;
   // clear backup infos
   _faninNodeBackup.clear();
   _faninPOBackup.clear();

   if (!isSat)
      _nodeSorted.clear();
   poIgr.clear();
}

/*
 * Compute the equivalent POs of old network and golden network
 * set-up _poDone
 */
void
IntEco::itp_lockPO(){
   #ifdef TIMING
   time_cec.start();
   #endif
   if ( _solverUse == ITP_WSAT ){
      for ( unsigned i = 0 ; i < _nPO ; ++i ){
         if ( _poDone[normalize(_poSet[i+_nPO])] )
            continue;
         AigNode eq = aigXor(_poSet[i],_poSet[i+_nPO]);
         AigNet net, netNew;
         buildNet( net, eq, false );
         net._poList.push_back(eq);
         netNew = dupNet(net,"");
         WSatMgr*  wsatChkPO = new WSatMgr;
         wsatChkPO->setVerbosity(false);
         netNew.addToWSat(*wsatChkPO);
         AigNode& po = netNew._poList[0];
         wsatChkPO->assignValue( po.getId(), 0, po.isNegEdge()?WSAT::l_false:WSAT::l_true, NULL, 0 );
         if ( !wsatChkPO->solve() )
            _poDone[normalize(_poSet[i+_nPO])] = 1;
         delete wsatChkPO;
      }
   }else{   // MiniSAT
      for ( unsigned i = 0 ; i < _nPO ; ++i ){
         if ( _poDone[normalize(_poSet[i+_nPO])] )
            continue;
         AigNode eq = aigXor(_poSet[i],_poSet[i+_nPO]);
         AigNet net, netNew;
         buildNet( net, eq, false );
         net._poList.push_back(eq);
         vector<unsigned>  backupId = itp_backupId(net);
         net.reassignId();
         Solver*  miniChkPO = new Solver();
         net.addToSolver(*miniChkPO);
         AigNode& po = net._poList[0];
         vec<Lit> assumps;
         assumps.push(Lit( po.getId(),  po.isNegEdge() ));
         if ( !miniChkPO->solve(assumps) )
            _poDone[normalize(_poSet[i+_nPO])] = 1;
         delete miniChkPO;
         itp_restoreId(net,backupId);
      }
   }
   #ifdef TIMING
   time_cec.stop();
   #endif
}

/*
 * Clear redundant infos for current status
 */
void
IntEco::itp_reset(){
   _nodeSorted.clear();
   _commonNodes.clear();
}

/*
 * set up "poIgr" and "candDup" after "cand" is chosen(itp_selNode)
 */
void
IntEco::itp_assNode( AigNode& cand, const vector<AigNode>& poAffected, vector<AigNode>& poIgr, vector<AigNode>& candDup ){
   unsigned id = cand.getId();
   unsigned interval = _onSet->getNumNode();
   int ids[2] = { id, id+interval };//TODO

   // make candidate nodes const1 and const0 and backup old fanins
   _faninNodeBackup.resize(2);
   for ( unsigned i = 0 ; i < 2 ; ++i ){
      AigNode n = _onoff->_nodeList[ids[i]];
      candDup.push_back(n);
      _faninNodeBackup[i].push_back(n.getFanin(0));
      _faninNodeBackup[i].push_back(n.getFanin(1));
      switch(i){
         case 0:
            n.setFanin(0,_const1A);    n.setFanin(1,_const1A.neg());
            break;
         case 1:
            n.setFanin(0,_const1B);    n.setFanin(1,_const1B);
            break;
         default:
            assert(0);
      }
   }

   // choose pos to be fixed
   for ( unsigned x = 0 ; x < _nPO ; ++x ){
      AigNode n = _poSet[x+_nPO];
      assert(!n.isVisited());
      unsigned i;
      for ( i = 0 ; i < poAffected.size() ; ++i )
         if ( n.getId() == poAffected[i].getId() )
            break;
      if ( i == poAffected.size() && !_poDone[normalize(n)] ){
         int ithPOs[2] = { x, x+_nPO };
         for ( unsigned ith = 0 ; ith < 2 ; ++ith ){
            AigNode nn = _poLimit[ithPOs[ith]];
            vector<AigNode> fs(2,AigNode());
            fs[0] = nn.getFanin(0);
            fs[1] = nn.getFanin(1);
            _faninPOBackup.push_back(fs);
            poIgr.push_back(nn);
            switch(ith){
               case 0://unEq
                  nn.setFanin(0,_const1A); nn.setFanin(1,_const1A);
                  break;
               case 1://unEq
                  nn.setFanin(0,_const1B); nn.setFanin(1,_const1B);
                  break;
               default:
                  assert(0);
            }
         }
      }
   }
}

/*
 * connect generated patch into the original network
 */
void
IntEco::itp_cntItpPIO( AigNet& patch, AigNet& target, AigNode& cand, int ithDup ){
   foreach( AigNode& n, patch._nodeList ){
      if ( !n.isPI() )  continue;
      unsigned ith = 0;
      foreach( AigNode& nn, _commonNodes ){
         if ( nn.getName().compare( n.getName() ) == 0 ){
            switch(ithDup){
               case 0:
                  {
                     patch.redirTo( n, nn, nn.isNegEdge()?AIG_INVERTING:AIG_NON_INVERTING );
                     //patch.redirTo( n, nn, n.isNegEdge()?AIG_INVERTING:AIG_NON_INVERTING );
                     patch._nodeList[n.getId()] = nn.isNegEdge()?nn.neg():nn;
                     break;
                  }
               case 1:
                  {
                     AigNode dupNN = _commonNodesDup[ith];
                     patch.redirTo( n, dupNN,dupNN.isNegEdge()?AIG_INVERTING: AIG_NON_INVERTING );
                     //patch.redirTo( n, dupNN,n.isNegEdge()?AIG_INVERTING: AIG_NON_INVERTING );
                     patch._nodeList[n.getId()] = dupNN.isNegEdge()?dupNN.neg():dupNN;
                     break;
                  }
            }
            break;
         }
         ++ith;
      }
   }
   patch._poList[0].setName(cand.getName());
   target.redirTo( cand, patch._poList[0], cand.isNegEdge()?AIG_INVERTING:AIG_NON_INVERTING );
   foreach( AigNode& po, _poSet ){
      if ( normalize(po) == normalize(cand) ){
         map<size_t,bool>::iterator it = _poDone.find(normalize(po));
         if ( it != _poDone.end() ){
            _poDone.erase(it);
         }
         //po = po.isNegEdge()^cand.isNegEdge() ? patch._poList[0] : patch._poList[0].neg() ;
         po = cand.isNegEdge() ? patch._poList[0] : patch._poList[0].neg() ;
         break;
      }
   }
}

/*
 * Find specified node from given node vector
 * isByName
 *    => true: searched by name
 *    => false:searched by address(size_t)
 */
AigNode
IntEco::itp_findNode( const AigNode& target, const vector<AigNode>& searchVec, bool isByName ){
   if ( isByName ){
      string name = target.getName();
      foreach( const AigNode& n, searchVec )
         if ( name.compare(n.getName()) == 0 )
            return n;
   }else{ // by size_t
      size_t s = normalize(target);
      foreach( const AigNode& n, searchVec )
         if ( s == normalize(n) )
            return n;
   }
   return AigNode(0);
}

/*
 * Specify to-be-used SAT solver
 */
void
IntEco::itp_setSolver( ITP_SOLVER s )   {  _solverUse = s; }

/*
 * Judge a node is candidate or not
 *    => return true: not candidate
 *    => return false: candidate
 */
bool
IntEco::itp_judge(){
   #ifdef TIMING
   time_sel.start();
   #endif
   bool rst;
   if ( _solverUse == ITP_WSAT ){
      AigNode out = _judgeCand[0] &  _judgeCand[1];
      out = out & _judgeCand[2];
      AigNet net;
      buildNet(net,out,false);
      net._poList.push_back(out);
      AigNet& netNew = net;
      WSatMgr w;
      w.setVerbosity(false);
      netNew.addToWSat(w);
      vector<WSatBitLit> assumps;
      for (unsigned idx = 0, n = netNew._nodeList.size(); idx < n; ++idx) {
         AigNode& c = netNew._nodeList[idx];
         if ((c() & WSatGate::FANIN_PTR_MASK) == (out() & WSatGate::FANIN_PTR_MASK))
            w.assignValue(idx, 0, WSAT::l_true, NULL, 0);
         if (c.isPI())
            if (c.getName().compare("const1A") == 0 || c.getName().compare("const1B") == 0)
               assumps.push_back(WSatBitLit(WSAT::l_true, idx, 0));
      }
      rst = w.solve(assumps);
   }else{
      AigNode out = _judgeCand[0] & _judgeCand[1];
      out = out & _judgeCand[2];
      AigNet net;
      buildNet(net,out,false);
      net._poList.push_back(out);
      AigNet& netNew = net;
      Solver s;
      vector<unsigned> backupId = itp_backupId(netNew);
      netNew.reassignId();
      netNew.addToSolver(s);
      vec<Lit> asmps;
      asmps.push(Lit(_const1A.getId(),0));
      asmps.push(Lit(_const1B.getId(),0));
      asmps.push(Lit(out.getId(),out.isNegEdge()));
      s._conflictNum = _onoff->getNumPI()/5;
      rst = s.solve(asmps);
      if ( s._aborted )
         rst=0;
      itp_restoreId(netNew,backupId);
   }
   #ifdef TIMING
   time_sel.stop();
   #endif
   return rst;
}

/*
 * Replace the selected node by a generate patch
 */
void
IntEco::itp_replace( vector<AigNode>& candDup, unsigned l){
   // adjust onset
   AigNode& cand = candDup[0];
   AigNet lastPatch = dupNet(_patchSet.back(),"");
   itp_cntItpPIO( lastPatch, *_onSet, cand, 0);
   unsigned id=0;
   vector<AigNode> tmpNodeList;
   for ( unsigned j = 0 ; j < _onSet->_nodeList.size() ; ++j ){
      AigNode& n = _onSet->_nodeList[j];
      if ( n() == cand() ){
         foreach ( AigNode& nn, _patchSet.back()._nodeList ){
            AigNode x = lastPatch._nodeList[nn.getId()];
            if ( nn.isPI() && itp_findNode(x,tmpNodeList,0)() != 0 )
               continue;
            tmpNodeList.push_back(x);
         }
         continue;
      }
      if ( l == 0 && n.isPI() && itp_findNode(n,tmpNodeList,0)() != 0 )
         continue;
      tmpNodeList.push_back(n);
   }
   foreach( AigNode& n, tmpNodeList )
      n.setId(id++);
   _onSet->_nodeList = tmpNodeList;

   // adjust offset
   cand = candDup[1];
   lastPatch = dupNet(_patchSet.back(),"");
   tmpNodeList.clear();
   itp_cntItpPIO( lastPatch, *_offSet, cand, 1);
   for ( unsigned j = 0 ; j < _offSet->_nodeList.size() ; ++j ){
      AigNode& n = _offSet->_nodeList[j];
      if ( n() == cand() ){
         foreach ( AigNode& nn, _patchSet.back()._nodeList ){
            AigNode x = lastPatch._nodeList[nn.getId()];
            if ( nn.isPI() && itp_findNode(x,tmpNodeList,0)() != 0 )
               continue;
            tmpNodeList.push_back(x);
         }
         continue;
      }
      if ( l == 0 && n.isPI() && itp_findNode(n,tmpNodeList,0)() != 0 )
         continue;
      tmpNodeList.push_back(n);
   }
   id = 0 ;
   foreach( AigNode& n, tmpNodeList )
      n.setId(id++);
   _offSet->_nodeList = tmpNodeList;
   itp_combineOnOff(0);
}

/*
 * Obtain interpolant
 * If SAT, no interpolant, return 1
 * If too large, backup the patch(_sp.limRatio), return -1 
 * If size is OK, return 0
 */
int
IntEco::itp_getItp(const AigNode& cand, const vector<AigNode>& poAffected, const vector<AigNode>& poIgr){
   #ifdef TIMING
   time_itp.start();
   #endif
   /* ---------- USE WSAT ---------------------
   WSatMgr wsat;
   wsat.setVerbosity(false);
   _onoff->addToWSat(wsat);
   // assign po const 1
   foreach ( const AigNode& po, _onoff->_poList ){
      if (!wsat.assignValue( po.getId(), 0,
               po.isNegEdge()?WSAT::l_false:WSAT::l_true, NULL, 0 ))
         assert(0);
   }
   // const1A=1 & const1B=1
   vector<WSatBitLit> assumps(2);
   assumps[0] = WSatBitLit(WSAT::l_true,_const1A.getId(),0);
   assumps[1] = WSatBitLit(WSAT::l_true,_const1B.getId(),0);
   WSatInt intMgr;
   intMgr.initial( &_mapVar, &_mapBase, &_onoff->_nodeList );
   // solve
   bool status = wsat.solve(assumps,&intMgr);
   -------------------------------------------- */

   ///* ----------- USE MINISAT -----------------
   Solver s;
   s.proof = new Proof();
   IntParam param;
   string proofname = "proof.eco";
   vec<Lit> assumps;
   //itp_toSolver_fast(s,param,assumps);
   itp_toSolver(s,param,assumps); 
   int status = (int)s.solve(assumps);
   //---------------------------------------------- */

   #ifdef TIMING
   time_itp.stop();
   #endif
   AigNet patch;
   if ( status==0 ){ //status==1:SAT, 0:UNSAT
      #ifdef TIMING
      time_itp.start();
      #endif
      if( _solverUse == ITP_WSAT ){
         //patch = intMgr.getItpNet(); //use WSAT
      }else{
         s.proof->save(proofname.c_str());
         param.filename = proofname;
         AigNode patch_po = Build_Interpolant(param);
         buildNet(patch,patch_po,1);
         patch._poList.push_back(patch_po);
         unlink(proofname.c_str());
         delete s.proof;
      }
      #ifdef TIMING
      time_itp.stop();
      time_opt.start();
      #endif
      //TODO
      AigNet newPatch = optimizeByABC(patch);
      newPatch.reassignId();
      //assert(cec(newPatch,patch));
      AigNode newPO = newPatch._poList[0];
      if ( newPO.isPI() ){ // adding dummy nodes for connecting
         AigNode node = newPO&newPO;
         node.setName("dummy_node");
         node.setId(newPatch.getNumNode());
         newPatch._nodeList.push_back(node);
         newPatch._poList[0] = node ;
      }
      patch.releaseAllNodes();
      #ifdef TIMING
      time_opt.stop();
      #endif
      /* Too large patch will be stored and find other patches */
      if ( _sp.limRatio < 1 && newPatch.getNumNode() > _onoff->getNumNode()*_sp.limRatio ){
         //itp_patchinfo_backup(newPatch,cand,poAffected,poIgr);
         newPatch.releaseAllNodes();
         status = -1;
      }else{
         _patchSet.push_back(newPatch);
         //_patchSize.push_back(patch.getNumNode()-patch.getNumPI());
      }
   }
   return status;
}

bool  sortMethod( pair<AigNode,vector<AigNode> > a, pair<AigNode,vector<AigNode> > b ) {
   if ( a.second.size() < b.second.size() )  return true;
   if ( a.second.size() == b.second.size() && a.first.getId() > b.first.getId() )   return true;
   return false;
}
bool  sortMethodId( AigNode a, AigNode b ) {
   if ( a.getId() < b.getId() )  return true;
   return false;
}
bool  sortMethodRev( pair<AigNode,vector<AigNode> > a, pair<AigNode,vector<AigNode> > b ) { return (a.second.size() > b.second.size()); }

map<size_t,bool>*  nameMap;
unsigned weight;
void  IntEco::itp_getNameMap() { nameMap = &_nameMap; weight = 2*_nPO; }
bool  sortMethodGrade( pair<AigNode,vector<AigNode> > a, pair<AigNode,vector<AigNode> > b ) {
   int gradeA = 0, gradeB = 0;
   int idLarger = a.first.getId() > b.first.getId();
   gradeA = a.second.size() + idLarger;
   gradeB = b.second.size();
   size_t aa = a.first()-a.first()%2;
   size_t bb = b.first()-b.first()%2;
   if ( nameMap->find(aa) != nameMap->end() )
      gradeA +=  weight*(*nameMap)[aa] ;
   if ( nameMap->find(bb) != nameMap->end() )
      gradeB +=  weight*(*nameMap)[bb] ;
   if ( gradeA <= gradeB )  return true;
   return false;
}

/*
 * Selecte a node from "_nodeSorted"
 * Set up "poAffected"
 */
void
IntEco::itp_selNode( AigNode& cand, vector<AigNode>& poAffected, unsigned ithLevel, AigNode& po,
      const SolveParameters& sp ){
   vector<AigNode> nodeSet;
   while(_poSorted.size()!=0){
      po = _poSorted.back().first;
      if ( _poDone.find(normalize(po))->second )   _poSorted.pop_back();
      else                   { nodeSet = _poSorted.back().second; break; }
   }
   if ( _nodeSorted.size() == 0 ){
      _nodeSorted.resize(nodeSet.size());
      for (unsigned i = 0 ; i < nodeSet.size() ; ++i ){
         pair<AigNode, vector<AigNode> > p = make_pair( nodeSet[i], _nodeInfo.find(normalize(nodeSet[i]))->second );
         _nodeSorted[i] = p;
      }
      if ( sp.isUseNameMapping ) sort( _nodeSorted.begin(), _nodeSorted.end(), sortMethodGrade );
      else                       sort( _nodeSorted.begin(), _nodeSorted.end(), sortMethod );
      _ithCand = _nodeSorted.size();
   }

   // search every nodes for _levels.back() and _levels[_levels.size()-2]
   /*
   if ( (unsigned)_ithCand != _nodeSorted.size() && (unsigned)ithLevel <= _levels.size() - 3 ){
      if ( (unsigned)_ithCand > _levels[ithLevel] )     _ithCand = _ithCand - _levels[ithLevel];
      else _ithCand = 0;
   }else  if ( ithLevel >= _levels.size() - 3 ){
      if ( (unsigned)_ithCand >= _levels[_levels.size()-3] )  _ithCand = _ithCand - _levels[_levels.size()-3];
      else _ithCand = 0;
   }else{  --_ithCand; }
   */
   int leapNum = 5;
   if ( _ithCand >= leapNum ){
      _ithCand -= leapNum;
   }else{  --_ithCand; }
   cand = _nodeSorted[_ithCand].first;
   if ( _mode == ONE_PO ){  
      poAffected.push_back(po);
      vector<AigNode>& pos = _nodeSorted[_ithCand].second;
      for ( unsigned i = 0 ; i < pos.size() ; ++i ){
         AigNode node_po = pos[i];
         if ( node_po.getId() != po.getId() ){
            if ( poAffected.size() >= _sp.fix_po_num )
               break;
            poAffected.push_back(node_po);
         }
      }
   }else{
      poAffected = _nodeSorted[_ithCand].second;
   }
}

/*
 * Return true: all POs are fixed
 * Return false: At least one PO is unfixed
 */
bool
IntEco::itp_checkPO(){
   for ( map<size_t,bool>::iterator it = _poDone.begin() ; it != _poDone.end() ; ++it )
      if ( !it->second )
         return 0;
   return 1;
}

/* ---  FUNCTION:  readNet-------------------------*//**
Read the original network from verilog format
----------------------------------------------------\n*/
void
IntEco::readNet( AigNet* aig ){
   _aigNet = aig;
   *_aigNetDup = dupNet(*aig, "_Dup");
}

/*
 * Set up gate types for WSAT
 */
void
IntEco::itp_setType(){
   _mapBase.clear();
   _mapVar.clear();
   _mapBase.resize(_onoff->_nodeList.size());
   // const 1
   _mapBase[_const1A.getId()] = BASE_A;               _mapBase[_const1B.getId()] = BASE_B;
   _mapVar[WSatBitVar(_const1A.getId(),0)] = VAR_A;   _mapVar[WSatBitVar(_const1B.getId(),0)] = VAR_B;
   foreach( const AigNode& n, _onSet->_nodeList ){
      _mapBase[n.getId()] = BASE_A;
      _mapVar[WSatBitVar(n.getId(),0)] = VAR_A;
   }
   foreach( const AigNode& n, _offSet->_nodeList ){
      _mapBase[n.getId()] = BASE_B;
      _mapVar[WSatBitVar(n.getId(),0)] = VAR_B;
   }
}

/* ---  FUNCTION  ------------------------------------
Name:  itp_build		Description:  Build all the circuit for circuit SAT based Interpolation method
----------------------------------------------------*/
void
IntEco::itp_build(){
   _nPO = (_aigNet->getNumPO() >> 1);
   itp_on ( *_aigNet );      // build up _onSet
   itp_off( *_aigNetDup );   // build up _offSet
   _const1A = AigNode::newPI();  _const1A.setName("const1A");  _const1A.setId(0);
   _const1B = AigNode::newPI();  _const1B.setName("const1B");  _const1B.setId(1);
   itp_combineOnOff(1);                  // build up _onoff
}

bool  sortId( AigNode a, AigNode b ) { return a.getId() < b.getId(); }

/* ---  FUNCTION: itp_combineOnOff---------------------*//**
combine onSet network and OffSet network
----------------------------------------------------\n*/
void
IntEco::itp_combineOnOff(bool addName ){
   *_onoff = *_onSet ;
   ostringstream s;
   _onoff->_nodeList.resize(_onSet->getNumNode()+2);
   for ( int i = _onoff->_nodeList.size()-1 ; i>=2 ; --i ){
      _onoff->_nodeList[i] = _onoff->_nodeList[i-2];
      _onoff->_nodeList[i].setId(i);
   }
   _onoff->_nodeList[0] = _const1A;
   _onoff->_nodeList[1] = _const1B;

   foreach( AigNode n, _offSet->_nodeList ){
      s.str("");
      s << n.getName();
      if ( addName ) s << "_Off";
      n.setName(s.str());
      n.setId(_onoff->_nodeList.size());
      _onoff->_nodeList.push_back(n);
   }
   _onoff->_poList[0] = _onSet->_poList[0];
   _onoff->_poList.push_back( _offSet->_poList[0]);
}

/* ---  FUNCTION: itp_off-------------------------*//**
Interpolation Off set
\n*///----------------------------------------------------
void
IntEco::itp_off( AigNet& orig ){
   foreach( AigNode& n, orig._poList )
      _poSet.push_back(n);
   vector<AigNode> limit = itp_poLimit( orig, OFFSET );
   foreach ( AigNode& n, limit )
      _poLimit.push_back(n);
   AigNode offsetOut = itp_poDynamicLimit( limit, 0);
   buildNet( *_offSet, offsetOut, true );
   _offSet->_poList.push_back(offsetOut);
}

/* ---  FUNCTION: itp_on-------------------------*//**
Interpolation On set
\n*///----------------------------------------------------
void
IntEco::itp_on( AigNet& orig ){
   foreach( AigNode& n, orig._poList )
      _poSet.push_back(n);
   vector<AigNode> limit = itp_poLimit( orig, ONSET );
   foreach ( AigNode& n, limit )
      _poLimit.push_back(n);
   AigNode onsetOut = itp_poDynamicLimit( limit, 1);
   buildNet( *_onSet, onsetOut, true );
   _onSet->_poList.push_back(onsetOut);
}


/* ---  FUNCTION: buildNet-----------------------*//**
Build network from POs
\n*///----------------------------------------------------
void
IntEco::buildNet( AigNet& net, AigNode& n, bool sortId ){
   map<size_t,bool> isTraced;
   buildRec( net, n, isTraced, sortId );
}


/* ---  FUNCTION: buildRec-----------------------*//**
Recursively build network from POs
\n*///----------------------------------------------------
void
IntEco::buildRec( AigNet& net, AigNode& n, map<size_t,bool>& isTraced, bool sortId ){
   if ( isTraced[n()-n.isNegEdge()] == 1 )
      return;
   isTraced[n()-n.isNegEdge()] = 1;
   if ( n.isPI() ){
      if ( sortId )
         n.setId(net._nodeList.size());
      net._nodeList.push_back(n.isNegEdge()?n.neg():n);
      return;
   }
   AigNode f0, f1;
   f0 = n.getFanin(0);
   f1 = n.getFanin(1);
   buildRec( net, f0, isTraced, sortId );
   buildRec( net, f1, isTraced, sortId );
   if ( sortId )
      n.setId(net._nodeList.size());
   net._nodeList.push_back(n);
}


/* ---  FUNCTION: dupRevisedNetNoPI-----------------*//**
Duplicate revised network
\n*///----------------------------------------------------
AigNet
IntEco::dupRevisedNetNoPI( AigNet& net ){
   AigNet netNew;
   ostringstream name;
   map<size_t, AigNode> id2Node;
   foreach( AigNode& n, net._nodeList ){
      AigNode p;
      if ( n.isPI() ){  // won't push pi into netNew
         id2Node[n.getId()] = n;
         continue;
      }
      if ( !n.isVisited() ) // to be checked
         continue;
      //set func
      AigNode f0 = id2Node[n.getFanin(0).getId()];
      AigNode f1 = id2Node[n.getFanin(1).getId()];
      f0 = n.getFanin(0).isNegEdge() ? f0.neg() : f0;
      f1 = n.getFanin(1).isNegEdge() ? f1.neg() : f1;
      p = f0 & f1;
      //set name
      name.str("");
      name << n.getName() << "_DupRev";
      p.setName(name.str());
      //set ID
      p.setId(netNew._nodeList.size());

      netNew._nodeList.push_back(p);
      id2Node[n.getId()] = p;
      if ( net.isPO(n) ){
         netNew._poList.push_back(p);
      }
   }
   return netNew;
}


/* ---  FUNCTION:  dupNet-------------------------*//**
Duplicate whole network
\n*///----------------------------------------------------
AigNet
IntEco::dupNet( AigNet& net, string s ){
   AigNet netNew;
   ostringstream name;
   map<size_t, AigNode> id2Node;
   foreach( const AigNode& n, net._nodeList ){
      AigNode p;
      if ( n.isPI() ){
         p = AigNode::newPI();
         p = n.isNegEdge() ? p.neg() : p;
         name.str("");
         name << n.getName() << s;
         p.setName(name.str());
         p.setId(netNew._nodeList.size());
         if(n.isVisited())
            p.setVisited();
         else
            p.unsetVisited();
         id2Node[n()-n.isNegEdge()] = p ;
         netNew._nodeList.push_back(p);
         if ( net.isPO(n) )
            netNew._poList.push_back(p);
         continue;
      }
      //set func
      AigNode f0 = id2Node[n.getFanin(0)()-n.getFanin(0).isNegEdge()];
      AigNode f1 = id2Node[n.getFanin(1)()-n.getFanin(1).isNegEdge()];
      if ( f0.isNegEdge() ^ n.getFanin(0).isNegEdge() )
         f0 = f0.neg();
      if ( f1.isNegEdge() ^ n.getFanin(1).isNegEdge() )
         f1 = f1.neg();
      p = f0 & f1;
      p = n.isNegEdge() ? p.neg() : p;
      //set name
      name.str("");
      name << n.getName() << s;
      p.setName(name.str());
      //set ID
      p.setId(netNew._nodeList.size());
      //set visited
      if(n.isVisited())
         p.setVisited();
      else
         p.unsetVisited();
      netNew._nodeList.push_back(p);
      id2Node[n()-n.isNegEdge()] = p;
      if ( net.isPO(n) )
         netNew._poList.push_back(p);
   }
   // poList negation fixed since it's may be different from _nodeList
   for ( unsigned i = 0 ; i < net.getNumPO() ; ++i ){
      if ( netNew._poList[i].isNegEdge() != net._poList[i].isNegEdge() )
         netNew._poList[i] = netNew._poList[i].neg();
      assert(netNew._poList[i].isNegEdge() == net._poList[i].isNegEdge() );
   }
   return netNew;
}


/* ---  FUNCTION: itp_poLimit ------------------------*//**
Return all equal and non-equal nodes
\n*///----------------------------------------------------
vector<AigNode>
IntEco::itp_poLimit( AigNet& net, bool setType ){
   vector<AigNode> limit, golden, revise;
   foreach ( const AigNode& po, net._poList ){
      if ( po.isVisited() )
         golden.push_back(po);
      else
         revise.push_back(po);
   }
   assert( golden.size() == revise.size() );
   size_t size = golden.size();
   switch (setType){
      case ONSET:
         for ( unsigned i = 0 ; i < size ; ++i ){
            AigNode unEq = aigXor( golden[i], revise[i] );
            limit.push_back(unEq);
         }
         break;
      case OFFSET:
         for ( unsigned i = 0 ; i < size ; ++i ){
            AigNode unEq = aigXor( golden[i], revise[i] );
            limit.push_back(unEq);
         }
         break;
      default:
         assert(0);
   }
   return limit;
}
/* ---  FUNCTION: itp_poDynamicLimit ------------------------*//**
Return all equal and non-equal nodes
\n*///----------------------------------------------------
AigNode
IntEco::itp_poDynamicLimit( vector<AigNode>& limit, bool isOn ){
   size_t npo = (limit.size());
   AigNode output = limit[0];
   // Make sure one of the assigned pos is non-equal
   for ( size_t i = 1 ; i < npo ; ++i )
      output = limit[i] | output;
   _judgeCand[!isOn] = output; //TODO
   return output;
}


/* ---  FUNCTION: aigXor----------------------------*//**
Perform XOR on aig structure
\n*///----------------------------------------------------
AigNode
IntEco::aigXor( const AigNode& a, const AigNode& b ) const{
   AigNode c1 = a & b.neg();
   AigNode c2 = b & a.neg();
   return ( c1 | c2 );
}

/* ---  FUNCTION: aigXnor----------------------------*//**
Perform XNOR on aig structure
\n*///----------------------------------------------------
AigNode
IntEco::aigXnor( const AigNode& a, const AigNode& b ) const{
   AigNode c1 = a.neg() & b.neg();
   AigNode c2 = a & b;
   return ( c1 | c2 );
}

/* ---  FUNCTION: itp_commonRec----------------------------*//**
Find global variables recursively according the specified level
\n*///----------------------------------------------------
void
IntEco::itp_commonRec( AigNode n, unsigned l, vector<AigNode>& gls, vector<bool>& traced ){
   if ( traced[n.getId()] )   return;
   traced[n.getId()] = 1;
   if ( n.isInternal() ){
      AigNode f0 = n.getFanin(0);//TODO
      itp_commonRec(f0,l,gls,traced);
      AigNode f1 = n.getFanin(1);
      itp_commonRec(f1,l,gls,traced);
      return;
   }
   if ( n.isPI() || l == 0 || n.isVisited() ){
      if ( n.getId() == _const1A.getId() || n.getId() == _const1B.getId() )
         return;
      gls.push_back( _onoff->_nodeList[n.getId()] );
      return;
   }
   AigNode f0 = n.getFanin(0);//TODO
   itp_commonRec(f0,l-1,gls,traced);
   AigNode f1 = n.getFanin(1);
   itp_commonRec(f1,l-1,gls,traced);
}

/* ---  FUNCTION: itp_common----------------------------*//**
Find global variables
\n*///----------------------------------------------------
void
IntEco::itp_common( AigNode cand, unsigned l ){
   //TODO
   vector<AigNode> gls;
   if ( l == 0 ){ //all PIs
      foreach( AigNode& n, _onSet->_nodeList )
         if ( n.isPI() )
            gls.push_back(n);
   }else{
      vector<bool> traced;
      traced.resize(2*_onSet->getNumNode(),0);
      itp_commonRec(cand,l,gls,traced);
   }
   for ( unsigned i = 0 ; i < gls.size() ; ++i )
      _mapVar[WSatBitVar(gls[i].getId(),0)] = VAR_GLOBAL;
   _commonNodes = gls;
   _commonNodesDup.clear();
   foreach( AigNode& n, _commonNodes )
      _commonNodesDup.push_back(_offSet->_nodeList[n.getId()-2]);

   vector<AigNode> piA = gls;
   vector<AigNode> piB = gls;
   // BASE_B VAR_B
   foreach ( AigNode& nn, gls )
      nn = aigXnor( nn, _onoff->_nodeList[_onSet->getNumNode()+ nn.getId()] );
   AigNode out = gls[0];
   for ( unsigned i = 1 ; i < gls.size() ; ++i )
      out = (gls[i] & out);
   _judgeCand[2] = out;
   // set common net
   _commonNet->_nodeList.clear();
   _commonNet->_poList.clear();
   buildNet( *_commonNet, out, false );
   _commonNet->_poList.push_back(out);
   foreach( AigNode& nn, _commonNet->_nodeList ){
      if ( nn.getName().length() == 0 ){
         nn.setId(_onoff->_nodeList.size());
         _onoff->_nodeList.push_back(nn);
         _mapBase.push_back(BASE_B);
         _mapVar[WSatBitVar(nn.getId(),0)] = VAR_B;
      }
   }
   out = out & _onoff->_poList[1];
   out.setId( _onoff->_nodeList.size() );
   _mapBase.push_back(BASE_B);
   _mapVar[WSatBitVar(out.getId(),0)] = VAR_B;
   _onoff->_nodeList.push_back(out);
   _onoff->_poList[1] = out;
}

void
IntEco::itp_sort( unsigned l ){
   _poSorted.resize(_nPO);
   for ( unsigned ithPO = 0 ; ithPO < _nPO ; ++ithPO ){
      AigNode n = _poSet[ithPO+_nPO];
      vector<bool> flag(_onoff->getNumNode(), 0);
      unsigned num = 0;
      _poSorted[ithPO].first = n;
      itp_sortRec( n, flag, num, n, ithPO );
   }
   sort(_poSorted.begin(),_poSorted.end(),sortMethodRev );
   //sort(_poSorted.begin(),_poSorted.end(),sortMethod );
}

void
IntEco::itp_sortRec( AigNode n, vector<bool>& flag, unsigned& num, AigNode po, unsigned ithPO ){
   if ( flag[n.getId()] ) return;
   if ( n.isVisited() )   return;
   flag[n.getId()] = 1;
   ++num;
   if ( n.isPI() )        return;
   if ( !n.isInternal() ){
      _poSorted[ithPO].second.push_back(n);
      _nodeInfo[normalize(n)].push_back(po);
   }
   itp_sortRec( n.getFanin(0), flag, num, po, ithPO);
   itp_sortRec( n.getFanin(1), flag, num, po, ithPO);
}

size_t   IntEco::normalize(AigNode n) const {   return (n()-n()%2); }

/*
 * Simulate to fast block non-candidate nodes
 * Dynamicly control the simulation times
 *    => if blocking one: set current ith simulation to new limit
 *    => if can't block : *1.1
 */
bool
IntEco::itp_judgeSim( unsigned& limit ){
   #ifdef TIMING
   time_sim.start();
   #endif
   uint64_t maxNum = ~0;
   bool rst=0;
   vector<uint64_t> value(_onoff->getNumNode(),maxNum );
   vector<unsigned> backupId = itp_backupId(*_onoff);
   _onoff->reassignId();
   if ( _onoff->getNumPI() < 10 ){
      limit = 5;
      _sp.simulationSlope = 1;
   }else if ( _onoff->getNumPI() < 30 ){
      limit = 20;
      _sp.simulationSlope = 1;
   }
   for ( unsigned i = 0 ; i < limit ; ++i ){
      foreach( AigNode& n, _onoff->_nodeList )
         if ( n.isPI() )
            value[n.getId()] = ( rand() & maxNum );
      value[_const1A.getId()] = maxNum;
      value[_const1B.getId()] = maxNum;
      vector<bool> flag(_onoff->getNumNode(),0);
      if ( (itp_sim(_onoff->_poList[0],flag,value) & itp_sim(_onoff->_poList[1],flag,value)) > 0 ){
         rst=true;
         _sim_success++; 
         _sim_total+=i;
         break;
      }
   }
   _sim_times++;
   if (rst==0){   
      _sim_total+=limit;
      if ( limit > 500 )     limit = 100;
      else                    limit = unsigned(limit*_sp.simulationSlope );
   }
   itp_restoreId(*_onoff,backupId);
   #ifdef TIMING
   time_sim.stop();
   #endif
   return rst;
}
uint64_t
IntEco::itp_sim( AigNode n, vector<bool>& flag, vector<uint64_t>& value ){
   unsigned id = n.getId();
   if ( n.isPI() || flag[id] )
      return (n.isNegEdge() ? ~value[id] : value[id]) ;
   flag[id] = 1;
   value[id] =  itp_sim(n.getFanin(0),flag,value) & itp_sim(n.getFanin(1),flag,value);
   return (n.isNegEdge() ? ~value[id] : value[id]);
}

vector<unsigned>
IntEco::itp_backupId( AigNet& n ){
   vector<unsigned> v;
   v.resize(n.getNumNode());
   for ( unsigned i = 0 ; i < n._nodeList.size() ; ++i )
      v[i] = n._nodeList[i].getId();
   return v;
}

void
IntEco::itp_restoreId( AigNet& n, const vector<unsigned>& v ){
   for ( unsigned i = 0 ; i < v.size() ; ++i )
      n._nodeList[i].setId(v[i]);
}

void
IntEco::itp_nameMapping(){
   unsigned nNode = _aigNet->getNumNode();
   bool diff;
   for ( unsigned i = 0 ; i < nNode ; ++i ){
      AigNode& n1 = _onSet->_nodeList[i];
      if ( n1.isVisited() )
         continue;
      for ( unsigned j = 0 ; j < nNode ; ++j ){
         AigNode& n2 = _onSet->_nodeList[j];
         if ( !n2.isVisited() )
            continue;
         if ( n1.getName() == n2.getName() ){   // check the structure of this gate
            if ( n1.isPI() && n2.isPI() )
               diff = 0;
            else if ( n1.isPI() ^ n2.isPI() )
               diff = 1;
            else
               diff =
                  (n1.getFanin(0).isNegEdge()^n2.getFanin(0).isNegEdge() ) |
                  (n1.getFanin(1).isNegEdge()^n2.getFanin(1).isNegEdge() ) |
                  (n1.getFanin(0).getName()!=n2.getFanin(0).getName() ) |
                  (n1.getFanin(1).getName()!=n2.getFanin(1).getName() ) ;
            _nameMap[normalize(n1)] = diff;
         }
      }
   }
   itp_getNameMap();
}

/*
 * Compute dominators for each nodes
 */
void
IntEco::itp_dominator(){
   /* traverse and store all dominators */
   #ifdef TIMING
   time_dom.start();
   #endif
   _dominator.resize(3*_onSet->getNumNode());
   vector<vector<unsigned> > dominated(3*_onSet->getNumNode());
   for ( int i = _onSet->getNumNode()-1 ; i >= 0; --i ){
      AigNode& n = _onSet->_nodeList[i];
      if ( n.isPI() || n.isVisited() )
         continue;
      AigNode fn0 = n.getFanin(0);
      AigNode fn1 = n.getFanin(1);
      if( !n.isInternal() )
         dominated[n.getId()].push_back(n.getId());
      itp_vecIntersect( dominated[n.getId()], dominated[fn0.getId()] );
      itp_vecIntersect( dominated[n.getId()], dominated[fn1.getId()] );
   }
   for ( unsigned i = 0 ; i < _onSet->getNumNode() ; ++i ){
      AigNode n = _onSet->_nodeList[i];
      if ( n.isVisited() )
         continue;
      for ( unsigned j = 0 ; j < dominated[n.getId()].size() ; ++j )
         _dominator[dominated[n.getId()][j]].push_back(n.getId());
   }
   #ifdef TIMING
   time_dom.stop();
   #endif
}
void
IntEco::itp_vecIntersect(const vector<unsigned>& s, vector<unsigned>& t ){
   if ( t.size() == 0 ){
      t = s;
      sort(t.begin(),t.end());
      return;
   }
   vector<unsigned> newT(s.size());
   vector<unsigned>::iterator it = set_intersection (s.begin(), s.end(), t.begin(), t.end(), newT.begin());
   newT.resize(it-newT.begin());
   t = newT;
}

/*
 * Extract current old network
 */
void
IntEco::itp_extractNet(AigNet& net){   // Id will be changed
   for ( unsigned i = 0 ; i < (_onSet->getNumNode()>>1) ; ++i ){
      AigNode n = _onSet->_nodeList[i];
      if ( n.getName().size() != 0 )
         net._nodeList.push_back(n);
      else{
         AigNode f0 = n.getFanin(0);
         if ( f0.getName().size() != 0 && !net.isPO(f0) )  net._poList.push_back(f0.neg());
         else                                              continue;
         AigNode f1 = n.getFanin(1);
         if ( f1.getName().size() != 0 )
            net._poList.push_back(f1.neg());
      }
   }
   net.reassignId();
}

/*
 * Wrapping function
 * Draw a specified network
 */
void
IntEco::itp_draw(AigNet& net, string fn ){
   net.drawNet("tmpfile.dot");
   string cmd = "dot -o ~/public_html/" + fn + " -Tpng tmpfile.dot";
   system(cmd.c_str());
   unlink("tmpfile.dot");
}

/*
 * Write a blif-format file for a specified network
 * Assume all POs and PIs have name
 */
void
IntEco::itp_wblif( AigNet& net, string fn ){
   ofstream ofile(fn.c_str());
   ofile << ".model patch.blif" << endl;
   ofile << ".inputs ";
   for ( unsigned i = 0 ; i < net._nodeList.size() ; ++i ){
      AigNode n = net._nodeList[i];
      if (n.isPI())
         ofile << n.getName() << " ";
   }
   ofile << endl;
   ofile << ".outputs ";
   for ( unsigned i = 0 ; i < net._poList.size() ; ++i ){
      AigNode n = net._poList[i];
      ofile << n.getName() << " ";
   }
   ofile << endl;
   vector<bool> traversed(net._nodeList.size(),0);
   for ( unsigned i = 0 ; i < net._poList.size() ; ++i ){
      AigNode n = net._poList[i];
      itp_wblif_rec(net,n,ofile,traversed);
   }
   ofile << ".end" << endl;
}
void
IntEco::itp_wblif_rec(AigNet& net, AigNode& n, ofstream& ofile, vector<bool>& traversed){
   if ( n.isPI() || traversed[n.getId()] )
      return;
   traversed[n.getId()] = 1;
   string tmp = "p_internal_";
   AigNode fn0 = n.getFanin(0);
   AigNode fn1 = n.getFanin(1);
   ofile << ".names ";
   bool isPO = net.isPO(n);
   if ( fn0.isPI() )         ofile << fn0.getName() << " ";
   else                      ofile << tmp << fn0.getName() << fn0.getId() << " ";
   if ( fn1.isPI() )         ofile << fn1.getName() << " ";
   else                      ofile << tmp << fn1.getName() << fn1.getId() << " ";
   if ( isPO )          ofile << n.getName() << " ";
   else                 ofile << tmp << n.getName() << n.getId() << " ";
   ofile << endl;
   ofile << !fn0.isNegEdge();
   ofile << !fn1.isNegEdge();
   if ( isPO && n.isNegEdge() )  
      ofile << " " << 0  << endl;
   else
      ofile << " " << 1 << endl;
   itp_wblif_rec(net,fn0,ofile,traversed);
   itp_wblif_rec(net,fn1,ofile,traversed);
}

/*
 * Combinational equivalence checking for two specified networks
 */
bool
IntEco::cec( AigNet& ntk1, AigNet& ntk2 ){
   ntk1.reassignId();
   //itp_draw(ntk1,"opt.png");
   //itp_draw(ntk2,"onoff.png");
   assert(ntk1.getNumPI()==ntk2.getNumPI());
   assert(ntk1.getNumPO()==ntk2.getNumPO());
   vector<AigNode> inputs1, inputs2;
   for ( unsigned i = 0 ; i < ntk1.getNumNode() ; ++i ){
      AigNode n = ntk1._nodeList[i];
      if ( n.getName()=="const1A"|| n.getName()=="const1B" )
         continue;
      if ( n.isPI() )
         inputs1.push_back(n);
   }
   for ( unsigned i = 0 ; i < ntk2.getNumNode() ; ++i ){
      AigNode n = ntk2._nodeList[i];
      if ( n.getName()=="const1A" || n.getName()=="const1B")
         continue;
      if ( n.isPI() )
         inputs2.push_back(n);
   }
   vector<AigNode> cons;
   for ( unsigned i = 0 ; i < inputs1.size() ; ++i ){
      AigNode c;
      if ( inputs1[i].isNegEdge()==inputs2[i].isNegEdge() )
         c = aigXnor(inputs1[i],inputs2[i]);
      else
         c = aigXor(inputs1[i],inputs2[i]);
      cons.push_back(c);
   }
   AigNode picons = cons[0];
   for ( unsigned i = 1 ; i < cons.size() ; ++i ){
      picons = picons & cons[i];
   }
   for ( unsigned i = 0 ; i < ntk2.getNumPO() ; ++i ){
      AigNode n = aigXor(ntk1._poList[i],ntk2._poList[i]);
      n = n & picons;
      AigNet newNet;
      buildNet(newNet,n,1);
      newNet._poList.push_back(n);
      //itp_draw(newNet,"nn.png");
      vec<Lit> lits;
      lits.push(toLit(2*n.getId()));
      Solver* s = new Solver();
      newNet.addToSolver(*s);
      if ( s->solve(lits) == 1 )
         return 0;
   }
   return 1;
}

/*
 * Collect fanin cone from the "ntk. _poList" into "rstntk"
 * Usage: remove redudant nodes or get whole network from given POs
 */
void
IntEco::itp_ntkClean(const AigNet& ntk, AigNet& rstntk, unsigned nNodes ){
   vector<bool> traversed;
   if (nNodes==0)    traversed.resize(ntk.getNumNode(),0);
   else              traversed.resize(nNodes,0);
   for ( unsigned i = 0 ; i < ntk.getNumPO() ; ++i ){
      AigNode po = ntk._poList[i];
      itp_ntkCleanRec(rstntk,po,traversed);
      rstntk._poList.push_back(po);
   }
}
void
IntEco::itp_ntkCleanRec( AigNet& ntk, AigNode n, vector<bool>& traversed ){
   if ( traversed[n.getId()] )
      return;
   if ( n.isPI() ){
      ntk._nodeList.push_back(n);
      traversed[n.getId()] = 1;
      return;
   }
   AigNode fn0 = n.getFanin(0);
   AigNode fn1 = n.getFanin(1);
   itp_ntkCleanRec(ntk,fn0,traversed);
   itp_ntkCleanRec(ntk,fn1,traversed);
   ntk._nodeList.push_back(n);
   traversed[n.getId()] = 1;
}

/*
 * Not for the general case, only for ECO
 */
void 
IntEco::itp_toSolver( Solver& s, IntParam& param, vec<Lit>& assumps ){
   AigNet cone, net;
   cone._poList.push_back(_judgeCand[0]);
   cone._poList.push_back(_onoff->_poList[1]);
   foreach ( AigNode n, _commonNodes )
      cone._poList.push_back(n);
   foreach ( AigNode n, _commonNodesDup )
      cone._poList.push_back(n);
   itp_ntkClean(cone,net,_onoff->getNumNode());

   vector<unsigned>  backupId = itp_backupId(net);
   net.reassignId();
   unsigned nNodes = net.getNumNode();
   for (unsigned i = 0; i < nNodes ; ++i)
      s.newVar();
   vector<bool> traversed(nNodes,0);
   unsigned nCommons = _commonNodes.size();
   itp_toSolverRec(net._poList[0],s,param,1,traversed);
   for ( unsigned i = 0 ; i < nCommons ; ++i )
      itp_toSolverRec(net._poList[i+2],s,param,1,traversed);
   for ( unsigned i = 0 ; i < nCommons ; ++i )
      itp_toSolverRec(net._poList[i+nCommons+2],s,param,0,traversed);
   itp_toSolverRec(net._poList[1],s,param,0,traversed);

   for ( unsigned i = 0; i < nCommons ; ++i ){
      AigNode n = _commonNodes[i];
      param.VarName.push_back(pair<unsigned int, string>(n.getId(),n.getName()));
   }
   assumps.growTo(4);
   assumps[0] = toLit(2*_const1A.getId());
   assumps[1] = toLit(2*_const1B.getId());
   assumps[2] = toLit(2*net._poList[0].getId()+net._poList[0].isNegEdge());
   assumps[3] = toLit(2*net._poList[1].getId()+net._poList[1].isNegEdge());
   
   itp_restoreId(net,backupId);
   param.nVar = nNodes;
}
void
IntEco::itp_toSolverRec( AigNode& n, Solver& s, IntParam& param, bool isVarA, vector<bool>& traversed ){
   if ( traversed[n.getId()] )
      return;
   traversed[n.getId()] =1;
   if ( n.isPI() )
      return;
   AigNode fn0 = n.getFanin(0);
   AigNode fn1 = n.getFanin(1);
   itp_toSolverRec(fn0,s,param,isVarA,traversed);
   itp_toSolverRec(fn1,s,param,isVarA,traversed);
   n.addToSolver(s);
   unsigned nAddC=3;
   if ( fn0.getId() == fn1.getId() )
      if ( fn0.isNegEdge()!=fn1.isNegEdge() )
         nAddC=2;
   for ( unsigned i = 0 ; i < nAddC ; ++i )
      param.isClauseA.push_back(isVarA);   
   traversed[n.getId()] = 1;
}

/*
 * Depth-first-search to build CNFs for given solver("s") and "param" and unit assumptions("assumps")
 */
void
IntEco::itp_toSolver_fast( Solver& s, IntParam& param, vec<Lit>& assumps){
   AigNode tmp = _judgeCand[1] &  _judgeCand[2];
   AigNode out = tmp & _judgeCand[0];
   AigNet net;
   buildNet(net,out,false);
   net._poList.push_back(out.getFanin(0));
   net._poList.push_back(out.getFanin(1));
   net._nodeList.pop_back();
   vector<unsigned>  backupId = itp_backupId(net);
   net.reassignId();
   vector<bool>      visiteds = itp_backup_visited(net);
   itp_set_cone_visited(net,_judgeCand[0],1);
   itp_set_cone_visited(net,tmp,0);
   for ( unsigned i = 0 ; i < _commonNodes.size() ; ++i )
      itp_set_cone_visited(net,_commonNodes[i],1);
   for ( unsigned i = 0 ; i < net.getNumNode() ; ++i ){
      AigNode n = net._nodeList[i];
      if ( n.isPI() )
         continue;
      AigNode fn0 = n.getFanin(0);
      AigNode fn1 = n.getFanin(1);
      unsigned nAddC=3;
      if ( fn0.getId() == fn1.getId() )
         if ( fn0.isNegEdge()!=fn1.isNegEdge() )
            nAddC=2;
      for ( unsigned i = 0 ; i < nAddC ; ++i )
         param.isClauseA.push_back(n.isVisited());   
   }
   for ( unsigned i = 0; i < _commonNodes.size() ; ++i ){
      AigNode n = _commonNodes[i];
      param.VarName.push_back(pair<unsigned int, string>(n.getId(),n.getName()));
   }
   param.nVar = net.getNumNode();
   assumps.growTo(4);
   assumps[0] = toLit(2*_const1A.getId());
   assumps[1] = toLit(2*_const1B.getId());
   assumps[2] = Lit(out.getFanin(0).getId(),out.getFanin(0).isNegEdge());
   assumps[3] = Lit(out.getFanin(1).getId(),out.getFanin(1).isNegEdge());
   net.addToSolver(s);
   itp_restoreId(net,backupId);
   itp_restore_visited(net,visiteds);
}

void IntEco::itp_patchinfo_clean() { _backupPatches.clear(); }
void IntEco::itp_patchinfo_backup(const AigNet& newPatch, const AigNode& cand,
                                  const vector<AigNode>& poAffected, const vector<AigNode>& poIgr )
{
   PatchInfo pinfo(newPatch,cand,_commonNodes,_commonNodesDup,_faninNodeBackup,_faninPOBackup,
                   poAffected,poIgr);
   _backupPatches.push_back(pinfo);
}

/*
 * Selece smallest patch from backups
 *    => if no backup, return true
 * Restore the selected patch info and prepare to replace its corresponding candidate
 */
bool IntEco::itp_patchinfo_restore(AigNode& cand, vector<AigNode>& poAffected, vector<AigNode>& poIgr){
   //if ( _backupPatches.size()==0 )
      return 1; // no small enough size
   unsigned size = _backupPatches[0]._patch.getNumNode();
   unsigned ith = 0;
   for ( unsigned i = 1 ; i < _backupPatches.size() ; ++i ){
      PatchInfo& pinfo = _backupPatches[i];
      if ( size > pinfo._patch.getNumNode() ){
         size = pinfo._patch.getNumNode();
         ith = i;
      }
   }
   PatchInfo& pinfo = _backupPatches[ith];
   _patchSet.push_back(pinfo._patch);
   _commonNodes = pinfo._commonNodes;
   _commonNodesDup = pinfo._commonNodesDup;
   _faninNodeBackup = pinfo._faninNodeBackup;
   _faninPOBackup = pinfo._faninPOBackup;
   poAffected = pinfo._poAffected;
   poIgr = pinfo._poIgr;
   cand = pinfo._cand;
   return 0;
}

/*
 * Follow the order of net._nodeList
 * net need to be reassigned id
 */
vector<bool> IntEco::itp_backup_visited( AigNet& net ){
   vector<bool> backupV(net.getNumNode(),0);
   for ( unsigned i = 0 ; i < net.getNumNode() ; ++i )
      backupV[i] = net._nodeList[i].isVisited();
   return backupV;
}

/*
 * Follow the order of net._nodeList
 * net need to be reassigned id
 */
void IntEco::itp_restore_visited( AigNet& net, const vector<bool>& visiteds ){
   assert(net.getNumNode()==visiteds.size());
   for ( unsigned i = 0 ; i < net.getNumNode() ; ++i ){
      if ( visiteds[i] )         net._nodeList[i].setVisited();
      else                       net._nodeList[i].unsetVisited();
   }
}

/*
 * Set while cone visited
 * net need to be reassigned id.
 */
void IntEco::itp_set_cone_visited(AigNet& net, AigNode& n, bool visited){
   vector<bool> traversed(net.getNumNode(),0);    
   itp_set_cone_visited_rec(n,visited,traversed);
}
void IntEco::itp_set_cone_visited_rec(AigNode& n, bool visited, vector<bool>& traversed ){
   if ( traversed[n.getId()] )
      return;
   traversed[n.getId()]  = 1;
   if (visited) n.setVisited();
   else         n.unsetVisited();
   if ( n.isPI() )
      return;
   AigNode fn0 = n.getFanin(0);
   AigNode fn1 = n.getFanin(1);
   itp_set_cone_visited_rec(fn0,visited,traversed);
   itp_set_cone_visited_rec(fn1,visited,traversed);
}
