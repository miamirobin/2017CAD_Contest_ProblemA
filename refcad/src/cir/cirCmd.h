/****************************************************************************
  FileName     [ cirCmd.h ]
  PackageName  [ cir ]
  Synopsis     [ Circuit commands ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CMD_CIRCMD_H_
#define CMD_CIRCMD_H_

#include "cmd/cmdBase.h"

NewCommand(CmdRead);
NewCommand(CmdWrite);
NewCommand(CmdPrint);
NewCommand(CmdSweep);
NewCommand(CmdOpt);
NewCommand(CmdStrash);
NewCommand(CmdSimulate);
NewCommand(CmdFraig);
NewCommand(CmdOptFull);

#endif  // CMD_CIRCMD_H_
