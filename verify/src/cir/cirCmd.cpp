/****************************************************************************
  FileName     [ cirCmd.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Circuit commands ]
  Author       [ music960633 ]
****************************************************************************/

#include "cirCmd.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

#include "cirMgr.h"

CirMgr* cirMgr = NULL;

CmdStatus CmdRead::exec(const std::vector<std::string>& args) {
  if (args.size() == 0)
    return CMD_MISSING_OPT;
  if (args.size() > 1)
    return CMD_EXTRA_OPT;
  if (cirMgr != NULL)
    delete cirMgr;
  cirMgr = new CirMgr();
  if (!cirMgr->readAig(args[0])) {
    delete cirMgr;
    cirMgr = NULL;
  }
  return CMD_OK;
}

CmdStatus CmdWrite::exec(const std::vector<std::string>& args) {
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  if (args.size() < 2)
    return CMD_MISSING_OPT;
  if (args.size() > 2)
    return CMD_EXTRA_OPT;
  if (args[0] != "-o")
    return CMD_ERROR_OPT;
  cirMgr->writeAig(args[1]);
  return CMD_OK;
}

CmdStatus CmdPrint::exec(const std::vector<std::string>& args) {
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  if (args.size() == 0) {
    cirMgr->report();
  } else if (args.size() == 1 && args[0] == "-n") {
    cirMgr->reportNetlist();
  } else {
    return CMD_ERROR_OPT;
  }
  return CMD_OK;
}

CmdStatus CmdSweep::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->sweep();
  return CMD_OK;
}

CmdStatus CmdOpt::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->optimize();
  return CMD_OK;
}

CmdStatus CmdStrash::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->strash();
  return CMD_OK;
}

CmdStatus CmdSimulate::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->simulate();
  return CMD_OK;
}

CmdStatus CmdFraig::exec(const std::vector<std::string>& args) {
  int effort = 1;
  bool setEffort = false;
  for (int i = 0, n = args.size(); i < n; ++i) {
    if (setEffort) return CMD_EXTRA_OPT;
    if (args[i] == "-low") {
      effort = 1;
      setEffort = true;
    } else if (args[i] == "-med") {
      effort = 10;
      setEffort = true;
    } else if (args[i] == "-high") {
      effort = 100;
      setEffort = true;
    } else if (args[i] == "-complete") {
      effort = -1;
      setEffort = true;
    } else {
      return CMD_ERROR_OPT;
    }
  }
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->fraig(effort);
  return CMD_OK;
}

CmdStatus CmdOptFull::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  if (cirMgr == NULL) {
    std::cerr << "Circuit is empty!" << std::endl;
    return CMD_OK;
  }
  cirMgr->optimize_full();
  return CMD_OK;
}
