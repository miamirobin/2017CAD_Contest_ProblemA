/****************************************************************************
  FileName     [ cmdMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Command manager class ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CMD_CMDMGR_H_
#define CMD_CMDMGR_H_

#include <string>
#include <map>

#include "cmdBase.h"

/**************************************
    Class CmdMgr definition
**************************************/
class CmdMgr {
 public:
  CmdMgr() {}
  ~CmdMgr();

  // cmdInit.cpp
  void initCommand();
  // cmdMgr.cpp 
  void addCommand(const std::string& cmdName, CmdBase* cmd);
  CmdStatus doCommand(const std::string& cmdStr);

 private:
  std::map<std::string, CmdBase*> cmdMap_;
};


#endif  // CMD_CMDMGR_H_
