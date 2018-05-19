/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ main function ]
  Author       [ music960633 ]
****************************************************************************/

#include <iostream>
#include <string.h>

#include "cmd/cmdParser.h"
#include "cir/cirMiter.h"
using namespace std;
int main(int argc, char** argv) {
  /*
  if (argc == 4) {
    CirMiter* miter = new CirMiter;
    miter->readVerilog(argv[1], argv[2]);
    miter->setOutputFile(argv[3]);
    miter->optimize_full();
    miter->solveNP3();
    delete miter;
  }
  */
	CirMiter* miter = new CirMiter;
   	miter->readVerilog(argv[1], argv[2]);
	miter->optimize_full();
        miter->solveNP3();
	delete miter;

  /*  CirMgr* mgr = new CirMgr;
    CirMgr* mgr2 = new CirMgr;
    //mgr->readVerilogandWriteAig(argv[1],argv[2]);
    mgr->readVerilog(argv[1]);
    
    mgr->reportAllPo();
    mgr2->readVerilog(argv[2]);
     mgr2->reportAllPo();
    //mgr->reportAigList() ;
    //mgr->reportTopoList();
    //mgr-> parse(argv[1]);
    //mgr->writeAig(argv[2]);

    
    */
    
    
    
    
/*  CmdParser cmdParser("fraig");
  if (argc == 1) {
    // command lines
    cmdParser.start();
  } else if (argc == 3 && !strcmp(argv[1], "-f")) {
    cmdParser.startDofile(argv[2]);
  }  */
    
  return 0;
}
