/****************************************************************************
  FileName     [ cmdBase.h ]
  PackageName  [ cmd ]
  Synopsis     [ Command base class ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CMD_CMDBASE_H_
#define CMD_CMDBASE_H_

#include <vector>
#include <string>

/**************************************
    Enum CmdStatus definition
**************************************/
enum CmdStatus {
  CMD_OK,
  CMD_NOT_FOUND,
  CMD_MISSING_OPT,
  CMD_EXTRA_OPT,
  CMD_ERROR_OPT,
  CMD_EXIT,

  CMD_TOT
};

/**************************************
    Class CmdBase definition
**************************************/
class CmdBase {
 public:
  CmdBase() {}
  virtual ~CmdBase() {};

  virtual CmdStatus exec(const std::vector<std::string>& args) = 0;
};

#define NewCommand(CmdClass)                            \
class CmdClass: public CmdBase {                        \
 public:                                                \
  CmdClass() {}                                         \
  ~CmdClass() {}                                        \
  CmdStatus exec(const std::vector<std::string>& args); \
}

#endif  // CMD_CMDBASE_H_
