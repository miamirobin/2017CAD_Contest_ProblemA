/****************************************************************************
  FileName     [ cmdMgr.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Command manager class ]
  Author       [ music960633 ]
****************************************************************************/

#include "cmdMgr.h"

#include <iostream>
#include <cstdlib>

#include "util/myString.h"

/**************************************
    Global variables
**************************************/
CmdMgr cmdMgr;

/**************************************
    CmdMgr member functions           
**************************************/
CmdMgr::~CmdMgr() {
  std::map<std::string, CmdBase*>::iterator it;
  for (it = cmdMap_.begin(); it != cmdMap_.end(); ++it)
    delete it->second;
}

void CmdMgr::addCommand(const std::string& cmdName, CmdBase* cmd) {
  if (cmdMap_.find(cmdName) != cmdMap_.end()) {
    std::cerr << "Command " << cmdName << " already exists!" << std::endl;
    exit(0);
  }
  cmdMap_[cmdName] = cmd;
}

CmdStatus CmdMgr::doCommand(const std::string& cmdStr) {
  std::vector<std::string> tokens = mySplit(cmdStr);
  if (tokens.size() == 0)
    return CMD_OK;
  if (cmdMap_.find(tokens[0]) == cmdMap_.end())
    return CMD_NOT_FOUND;
  CmdBase* cmd = cmdMap_[tokens[0]];
  tokens.erase(tokens.begin());
  return cmd->exec(tokens);
}
