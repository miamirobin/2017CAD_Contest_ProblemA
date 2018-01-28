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
  
  oriaigList_=aigList_;
  oritopoList_=topoList_;
//buldBFSList();
  return true;
}


void CirMgr::readWeight(const std::string& filename){
  std::ifstream ifs(filename.c_str());
  std::string str;
  std::vector<std::string> tokens;
 
  while (!ifs.eof()) {
    str = readUntil(ifs,'\n');
    tokens = split(str, " ,;()\t\r");
    std::string name=tokens[0];
    if (name!="\n"){
        CirGate* gate=getGateByName(name);   
        gate->setWeight( std::stoi(tokens[1]));
        if (bestWeight<=0){bestWeight=gate->getWeight();}
         else {if (gate->getWeight()<bestWeight){bestWeight=gate->getWeight();}}
    }
  } 
  ifs.close();               
  
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



bool CirMgr::removeComment(const std::string& inFilename, const std::string& outFilename) const {
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
  const0->setWeight(0);
  CirGate* const1 = addAIGate("1'b1");
  const1->setWeight(0);
  connectNot(const1, const0);
  while (!ifs.eof()) {
    str = readUntil(ifs, ';');
    tokens = split(str, " ,;()\t\r\n");
    unsigned n = tokens.size();
    if (n == 0) continue;
    
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
      for (unsigned i = 1; i < n; ++i)
        addAIGate(tokens[i]);
    }
      else if ((tokens[0] == "wire") and(tokens[1]=="t_0")) {
      // wire t_0 t_1...
      assert(n >= 2);
      for (unsigned i = 1; i < n; ++i){
        addWtGate(tokens[i]);
        //addAIGate(tokens[i]);
        //CirGate *out, *in0, *in1;
        //in0 = name2GateMap_["1'b0"];
        //in1 = name2GateMap_["1'b0"];
        //out = name2GateMap_[tokens[i]];
        //connect2inputGate("and", out, in0, in1);
      }
        
    } else if (tokens[0] == "endmodule") {
      // endmodule
      break;
    } else if(tokens[0] == "buf" || tokens[0] == "not") {
      // buf / not
      assert(n == 3);
      CirGate* out = name2GateMap_[tokens[1]]; 
      out->setnoinput();
      CirGate* in0 = name2GateMap_[tokens[2]]; 
      if (tokens[2]=="1'b0" or tokens[2]=="1'b1"){constcount++;}
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
      if (tokens[2]=="1'b0" or tokens[2]=="1'b1"){constcount++;}
      in1 = name2GateMap_[tokens[3]];                    
      if (tokens[3]=="1'b0" or tokens[3]=="1'b1"){constcount++;}
      for (unsigned i = 4; i < n; ++i) {
        out = addAIGate("");
        connect2inputGate(type, out, in0, in1);
        in0 = out;            
        in1 = name2GateMap_[tokens[i]];
      }
      out = name2GateMap_[tokens[1]];    out->setnoinput();
      connect2inputGate(tokens[0], out, in0, in1);    
    }
  }
  ifs.close();
  return true;
}












void CirMgr::writePatch(const std::string& filenamef, const std::string& filename,const std::string& filename1){
    //initiall
   
    int n=Patch.size();
    
 
    //write out.v
    std::cout<<"write out.v\n";     
    std::ifstream ifs1(filenamef.c_str());
    std::ofstream ofs1(filename1.c_str());
    
    std::string str;
    std::vector<std::string> tokens;
    
    while (!ifs1.eof()) {
       str = readUntil(ifs1,'\n');
       if (str=="\n"){
         ofs1<<std::endl;
         continue;
       }
     
       tokens = split(str, " \t\r\n");
      
       if (tokens[0]=="endmodule"){
          ofs1<<"patch p0 (";
          for (unsigned i = 0; i < output.size(); ++i){
               ofs1<<"."<<output[i]->getNAME()<<"("<<output[i]->getName()<<"), ";  
           }
           for (unsigned i = 0; i < cutlist.size(); ++i){
              if (i+1<cutlist.size()){
          	      ofs1 << "."<< cutlist[i]->getNAME()<<"("<<cutlist[i]->getName()<<")"<<", " ;
             }
             else {
                      ofs1<< "."<<cutlist[i]->getNAME()<<"("<<cutlist[i]->getName()<<"));"<<std::endl;
             } 
           }
       }
       for (int i=0;i<tokens.size();i++){
           ofs1<<tokens[i]<<" ";
       }
       ofs1<<std::endl;
   } 
   ifs1.close();  
   ofs1.close();



    //write patch.v
    std::cout<<"write patch.v\n";
    std::ofstream ofs(filename.c_str());
    
    // write module patch
  ofs <<"module patch (";        
  for (unsigned i = 0; i <output.size(); ++i){
    ofs<<output[i]->getNAME()<<", ";
  }
  for (unsigned i = 0; i < cutlist.size(); ++i){
    if (i+1<cutlist.size()){
    	ofs << cutlist[i]->getNAME()<<", " ;
    }
    else {
  	ofs<< cutlist[i]->getNAME()<<");"<<std::endl;
    }
  }
  // write input
  if(cutlist.size()>0) { ofs<<"input " ;}
  for (unsigned i = 0; i < cutlist.size(); ++i){
    if (i+1<cutlist.size()){
    	ofs << cutlist[i]->getNAME()<<", " ;
    }
    else {
  	ofs<< cutlist[i]->getNAME()<<";"<<std::endl;
    }
  }
  // write output
  if (output.size()>0){ofs<<"output " ;}
  for (unsigned i = 0; i < output.size(); ++i){
    if (i+1<output.size()){
    	ofs<<output[i]->getNAME()<<", ";
    }
    else {
  	ofs<<output[i]->getNAME()<<";"<<std::endl;
    }
  }
  // write wire
  
  if (wire.size()>0){ofs<<"wire ";}  
  for (unsigned i = 0; i < wire.size(); ++i){
   
    if (i+1<wire.size()){
    	ofs<<wire[i]->getNAME()<<", ";
    }
    else {
  	ofs<<wire[i]->getNAME()<<";"<<std::endl;
    }
   
  }
  ofs<<"\n";
  // write AIG
   for (int i=Patch.size()-1;i>=0;--i){
      if (Patch[i].size()==4){
           CirGate* gate=getGateByName(Patch[i][1]);
            CirGate* in0=getGateByName(Patch[i][2]);
             CirGate* in1=getGateByName(Patch[i][3]);
          ofs<<Patch[i][0]<<" ( "<<gate->getNAME()<<" , "<<in0->getNAME()<<" , "<<in1->getNAME()<<" );\n";

      }
      else {
              CirGate* gate=getGateByName(Patch[i][1]);
              CirGate* in0=getGateByName(Patch[i][2]);       
             ofs<<Patch[i][0]<<" ( "<<gate->getNAME()<<" , "<<in0->getNAME()<<" );\n";
      }


   }


 
  ofs<<"\n";
  // write endmodule
  ofs << "endmodule" << std::endl;
  ofs.close();
   
}





