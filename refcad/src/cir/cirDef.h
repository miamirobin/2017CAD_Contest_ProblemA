/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Circuit definitions ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef CIR_CIRDEF_H_
#define CIR_CIRDEF_H_

#include <vector>

/**************************************
    Forward declaration
**************************************/
class CirGate;
class CirGateV;

/**************************************
    Type definitions
**************************************/
// List of Gates
typedef std::vector<CirGate*>    GateList;
typedef std::vector<CirGateV>    GateVList;

// List of FEC
typedef std::vector<CirGateV>    FecGrp;
typedef std::vector<FecGrp*>     FecGrpList;

#endif  // CIR_CIRDEF_H_
