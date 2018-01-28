/****************************************************************************
  FileName     [ cmdParser.h ]
  PackageName  [ cmd ]
  Synopsis     [ Command parser class ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CMD_CMDPARSER_H_
#define CMD_CMDPARSER_H_

#include <vector>
#include <string>
#include <map>

#include "cmdMgr.h"

/**************************************
    Class CmdParser definition
**************************************/
class CmdParser {
 public:
  CmdParser(const std::string& prompt): prompt_(prompt) {
    cmdMgr_.initCommand();
  }
  ~CmdParser() {};

  bool doCommand(const std::string& cmdStr);
  void start();
  void startDofile(const std::string& filename);

 private:
  std::string prompt_;
  CmdMgr      cmdMgr_;
};


#endif  // CMD_CMDPARSER_H_
