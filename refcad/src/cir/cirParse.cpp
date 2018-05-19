/****************************************************************************
  FileName     [ cirParse.cpp ]
  PackageName  [ cir ]
  Synopsis     [ parse Verilog/AIGER function ]
  Author       [ music960633 ]
****************************************************************************/

#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#include "cir/cirMgr.h"
#include "util/parse.h"

bool CirMgr::readAig(const std::string& filename) {
  std::ifstream ifs(filename.c_str());
  if(ifs.fail()) {
    std::cerr << "Cannot open file \"" << filename << "\"!" << std::endl;
    return false;
  }
  std::string str;
  unsigned M, I, L, O, A;
  unsigned val, in0, in1;
  ifs >> str >> M >> I >> L >> O >> A;
  assert(str == "aag");
  assert(L == 0);
  init();
  maxId_ = M;
  GateList vec;
  vec.push_back(addConstGate("0"));
  // construct PI and AIG
  vec.resize(M+1, NULL);
  for (unsigned i = M+1; i <= M+O; ++i) {
    CirGate* poGate = addPoGate(myToString(i));
    poGate->setId(i);
    vec.push_back(poGate);
  }
  // PI
  for (unsigned i = 0; i < I; ++i) {
    ifs >> val;
    assert((val&1) == 0);
    CirGate* piGate = addPiGate(myToString(val >> 1));
    piGate->setId(val >> 1);
    vec[val >> 1] = piGate;
  }
  // AIG
  for (unsigned i = 1; i <= M; ++i) {
    if(vec[i] == NULL) {
      CirGate* aiGate = addAIGate(myToString(i));
      aiGate->setId(i);
      vec[i] = aiGate;
    }
  }
  // PO
  for (unsigned i = M+1; i <= M+O; ++i) {
    ifs >> val;
    vec[i]->setFanin(CirGateV(vec[val >> 1], val & 1));
    vec[val >> 1]->addFanout(CirGateV(vec[i], val & 1));
  }
  // AIG
  for (unsigned i = 0; i < A; ++i) {
    ifs >> val >> in0 >> in1;
    assert((val&1) == 0);
    vec[val >> 1]->setFanin(CirGateV(vec[in0 >> 1], in0 & 1),
                            CirGateV(vec[in1 >> 1], in1 & 1));
    vec[in0 >> 1]->addFanout(CirGateV(vec[val >> 1], in0 & 1));
    vec[in1 >> 1]->addFanout(CirGateV(vec[val >> 1], in1 & 1));
  }
  ifs.close();
  buildTopoList();
  return true;
}

bool CirMgr::readVerilog(const std::string& filename) {
  std::string tmpFilename = filename + ".tmp";
  if (!removeComment(filename, tmpFilename)) return false;
  if (!parse(tmpFilename)) return false;
  buildAigList();
  buildTopoList();
  return true;
}

bool CirMgr::readVerilogandWriteAig(const std::string& filename,const std::string& filename1) {
    std::string tmpFilename = filename + ".tmp";
    if (!removeComment(filename, tmpFilename)) return false;
    if (!parse(tmpFilename)) return false;
    buildAigList();
    buildTopoList();
    
    std::ofstream ofs(filename1.c_str());
    unsigned id = 0;
    unsigned numTot = topoList_.size();  // includes const0 gate
    unsigned numPi = piList_.size();
    unsigned numPo = poList_.size();
    unsigned numAig = aigList_.size();
    unsigned maxId = (maxId_ == 0 ? numTot - 1 : maxId_);
    assert(numTot = numPi + numPo + numAig + 1);
    // set gate id
    if (maxId_ == 0) {
        // set constant 0
        const0_->setId(id++);
        // set input
        for (unsigned i = 0; i < numPi; ++i)
            piList_[i]->setId(id++);
        // set AIG
        for (unsigned i = 0; i < numAig; ++i)
            aigList_[i]->setId(id++);
    }
    // write header
    ofs << "aag " << maxId << " " << numPi << " " << 0 << " "
    << numPo << " " << numAig << std::endl;
    // write PI
    for (unsigned i = 0; i < numPi; ++i)
        ofs << piList_[i]->getId()*2 << std::endl;
    // write PO
    for (unsigned i = 0; i < numPo; ++i) {
        CirGateV in0 = poList_[i]->getFanin0();
        ofs << in0.gate()->getId()*2 + (in0.isInv() ? 1 : 0) << std::endl;
    }
    // write AIG
    for (unsigned i = 0; i < numAig; ++i) {
        CirGateV in0 = aigList_[i]->getFanin0();
        CirGateV in1 = aigList_[i]->getFanin1();
        ofs << aigList_[i]->getId()*2 << " "
        << in0.gate()->getId()*2 + (in0.isInv() ? 1 : 0) << " "
        << in1.gate()->getId()*2 + (in1.isInv() ? 1 : 0) << std::endl;
    }
    // write comment
    ofs.close();

    return true;
}



bool CirMgr::removeComment(const std::string& inFilename,
                           const std::string& outFilename) const {
  std::ifstream ifs(inFilename.c_str());
  // cannot open file
  if (ifs.fail()) {
    std::cerr << "Cannot open file \"" << inFilename << "\"!" << std::endl;
    return false;
  }
  std::ofstream ofs(outFilename.c_str());
  // cannot open file
  if (ofs.fail()) {
    std::cerr << "Cannot open file \"" << outFilename << "\"!" << std::endl;
    return false;
  }
  // start reading
  char c, prev = '\0';
  bool lineCommentFlg = false, blockCommentFlg = false;
  while (!ifs.eof()) {
    ifs.get(c);
    // inside line comment
    if (lineCommentFlg) {
      if (c == '\n') {
        lineCommentFlg = false;
        ofs << std::endl;
      }
    } else if (blockCommentFlg) {
      if (prev == '*' && c == '/')
        blockCommentFlg = false;
    } else if (c == '/' && ifs.peek() == '/') {
      lineCommentFlg = true;
    } else if (c == '/' && ifs.peek() == '*') {
      blockCommentFlg = true;
    } else {
      ofs << c;
    }
    prev = c;
  }
  ifs.close();
  ofs.close();
  return true;
}

bool CirMgr::parse(const std::string& filename) {
  std::ifstream ifs(filename.c_str());
  std::string str;
  std::vector<std::string> tokens;
  init();
  // set constant 0 and 1
  CirGate* const0 = addConstGate("1'b0");
  CirGate* const1 = addAIGate("1'b1");
  connectNot(const1, const0);
  while (!ifs.eof()) {
    str = readUntil(ifs, ';');
    tokens = split(str, " ,;()\t\r\n");
    unsigned n = tokens.size();
    if (n == 0) continue;
    std::cout<<tokens[0]<<std::endl;
    if (tokens[0] == "module") {
      // module <module name> <name0> <name1> ...
      assert(n >= 2);
      setName(tokens[1]);
    } else if(tokens[0] == "input") {
      // input <name0> <name1> ...
      assert(n >= 2);
      for (unsigned i = 1; i < n; ++i)
        addPiGate(tokens[i]);
    } else if(tokens[0] == "output") {
      // output <name0> <name1> ...
      assert(n >= 2);
      for (unsigned i = 1; i < n; ++i) {
        CirGate* poGate = addPoGate(tokens[i]);
        CirGate* aiGate = addAIGate(tokens[i]);
        connectPo(poGate, aiGate);
      }
    } else if ((tokens[0] == "wire") and(tokens[1]!="t_0")) {
      // wire <name0> <name1> ...
      assert(n >= 2);
      for (unsigned i = 1; i < n; ++i){
        addAIGate(tokens[i]);
        std::cout<<tokens[i];
      }
    }
      else if ((tokens[0] == "wire") and(tokens[1]=="t_0")) {
      // wire t_0 t_1...
      assert(n >= 2);
      for (unsigned i = 1; i < n; ++i){
      	addPiGate(tokens[i]);
      }
        
    } else if (tokens[0] == "endmodule") {
      // endmodule
      break;
    } else if(tokens[0] == "buf" || tokens[0] == "not") {
      // buf / not
      assert(n == 3);
      CirGate* out = name2GateMap_[tokens[1]];
      CirGate* in0 = name2GateMap_[tokens[2]];
      connect1inputGate(tokens[0], out, in0);
    } else if(tokens[0] == "and" || tokens[0] == "nand" ||
              tokens[0] == "or"  || tokens[0] == "nor"  ||
              tokens[0] == "xor" || tokens[0] == "xnor" ) {
      // and / nand / or / nor / xor / xnor
      assert(n >= 4);
      std::string type = tokens[0];
      if (type == "nand") type = "and";
      if (type == "nor")  type = "or";
      if (type == "xnor") type = "xor";
      CirGate *out, *in0, *in1;
      in0 = name2GateMap_[tokens[2]];
      in1 = name2GateMap_[tokens[3]];
      for (unsigned i = 4; i < n; ++i) {
        out = addAIGate("");
        connect2inputGate(type, out, in0, in1);
        in0 = out;
        in1 = name2GateMap_[tokens[i]];
      }
      out = name2GateMap_[tokens[1]];
      connect2inputGate(tokens[0], out, in0, in1);
    }
  }
  ifs.close();
  return true;
}
