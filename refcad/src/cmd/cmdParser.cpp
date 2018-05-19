/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Command parser class ]
  Author       [ music960633 ]
****************************************************************************/

#include "cmdParser.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "cmdBase.h"

bool CmdParser::doCommand(const std::string& cmdStr) {
  switch (cmdMgr_.doCommand(cmdStr)) {
    case CMD_OK:
      break;
    case CMD_NOT_FOUND:
      std::cerr << "Cannot find command" << std::endl;
      break;
    case CMD_MISSING_OPT:
      std::cout << "Missing option" << std::endl;
      break;
    case CMD_EXTRA_OPT:
      std::cout << "Extra option" << std::endl;
      break;
    case CMD_ERROR_OPT:
      std::cout << "Error option" << std::endl;
      break;
    case CMD_EXIT:
      return false;
    default:
      std::cerr << "Something is wrong" << std::endl;
      return false;
  }
  return true;
}

void CmdParser::start() {
  std::string cmdStr;
  while (1) {
    std::cout << prompt_ << "> ";
    std::getline(std::cin, cmdStr);
    if (!doCommand(cmdStr)) break;
  }
}

void CmdParser::startDofile(const std::string& filename) {
  std::string cmdStr;
  std::ifstream ifs(filename.c_str());
  if (!(ifs.is_open())) {
    std::cerr << "Cannot open file!" << std::endl;
    return;
  }
  while (!ifs.eof()) {
    std::getline(ifs, cmdStr);
    std::cout << prompt_ << "> " << cmdStr << std::endl;
    if (!doCommand(cmdStr)) break;
  }
  ifs.close();
}
