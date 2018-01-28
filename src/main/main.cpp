/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ main function ]
  Author       [ sam031023 ]
****************************************************************************/

#include <iostream>
#include <string.h>
#include <time.h>
#include "cmd/cmdParser.h"
#include "cir/satMgr.h"
#include "cir/cirMiter.h"

using namespace std;

bool _globalVerbosityFlg = true;

int main(int argc, char** argv) {
  /*
  if (argc == 4) {
    CirMiter* miter = new CirMiter;
    miter->readVerilog(argv[1], argv[2]); // first is golden and second is revised one
    miter->setOutputFile(argv[3]);
    miter->optimize_full();
    miter->solveNP3();
    delete miter;
  }
  */


  if (_globalVerbosityFlg)
		cout << "Run with argv: " << argv[1]<<"  " <<argv[2] <<  endl;
	SATMgr* _mgr = new SATMgr();
	_mgr->readVerilog(argv[2], argv[1], argv[3]);    
        _mgr->build();
        _mgr->writepatch(argv[1],argv[4],argv[5]);

    



/*  CmdParser cmdParser("fraig");
  if (argc == 1) {
    // command lines
    cmdParser.start();
  } else if (argc == 3 && !strcmp(argv[1], "-f")) {
    cmdParser.startDofile(argv[2]);
  }  */
  cout << (double)clock() / CLOCKS_PER_SEC << " S"<<endl;
  return 0;
}
