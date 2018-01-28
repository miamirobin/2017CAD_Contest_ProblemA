/****************************************************************************
  FileName     [ cmdCommon.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Common command class ]
  Author       [ music960633 ]
****************************************************************************/

#include "cmdCommon.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

#include "cmdBase.h"
#include "util/myUsage.h"

MyUsage myUsage;

CmdStatus CmdQuit::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  else
    return CMD_EXIT;
}

CmdStatus CmdUsage::exec(const std::vector<std::string>& args) {
  if (args.size() > 0)
    return CMD_EXTRA_OPT;
  myUsage.report(true, true);
  return CMD_OK;
}

