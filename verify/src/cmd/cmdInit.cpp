/****************************************************************************
  FileName     [ cmdInit.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Initialize commands ]
  Author       [ music960633 ]
****************************************************************************/

#include "cmdMgr.h"

#include "cmdCommon.h"
#include "cir/cirCmd.h"

void CmdMgr::initCommand() {
  addCommand("usage"      , new CmdUsage);
  addCommand("quit"       , new CmdQuit);
  addCommand("cirread"    , new CmdRead);
  addCommand("cirwrite"   , new CmdWrite);
  addCommand("cirprint"   , new CmdPrint);
  addCommand("cirsweep"   , new CmdSweep);
  addCommand("ciropt"     , new CmdOpt);
  addCommand("cirstrash"  , new CmdStrash);
  addCommand("cirsim"     , new CmdSimulate);
  addCommand("cirfraig"   , new CmdFraig);
  addCommand("ciroptfull" , new CmdOptFull);
}
