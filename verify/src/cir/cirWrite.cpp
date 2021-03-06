/****************************************************************************
  FileName     [ cirWrite.cpp ]
  PackageName  [ cir ]
  Synopsis     [ write circuit ]
  Author       [ music960633 ]
****************************************************************************/

#include <fstream>
#include <string>
#include <cassert>
#include <map>
#include <string>
#include "cirMgr.h"
#include <vector>


void CirMgr::writeAig(const std::string& filename) {
  std::ofstream ofs(filename.c_str());
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
  ofs << "c" << std::endl;
  ofs << "generated by music960633" << std::endl;
  ofs.close();
}











void CirMgr::writePatch(const std::string& filenamef, const std::string& filename,const std::string& filename1){
    //initiall
    
    unsigned tmp=0;
    unsigned n =t0topolist.size();
    vector<std::string> pi;
    unsigned numT = tList_.size();  
    for (int i=0;i<n;i++){
       
       if (!t0topolist[i]->getFanin0().isNull()and!t0topolist[i]->getFanin1().isNull()){
              CirGateV in0 = t0topolist[i]->getFanin0();
              CirGateV in1 = t0topolist[i]->getFanin1();
              if (in0.isInv()){
                     CirGate* tmpGate = addAIGate("tmp"+to_string(tmp));
                     tmp++;
                     connectNot(tmpGate,in0.gate());
                     t0topolist[i]->setFanin0(CirGateV (tmpGate));
                     tmpGate->addFanout(CirGateV (t0topolist[i]));
              }
              if (in1.isInv()){
                     CirGate* tmpGate = addAIGate("tmp"+to_string(tmp));
                     tmp++;
                     connectNot(tmpGate,in1.gate());
                     t0topolist[i]->setFanin1(CirGateV (tmpGate));
                     tmpGate->addFanout(CirGateV (t0topolist[i]));
              }
                  
       }
       else if(t0topolist[i]->getFanin0().isNull()and t0topolist[i]->getFanin1().isNull()){
             pi.push_back(  t0topolist[i]->getName());
       }
    } 
    
    buildt0TopoList();
    n =t0topolist.size();   
    unsigned count=1;
    for (int i=0;i<n-1;i++){
       if (!t0topolist[i]->getFanin0().isNull()){
           std::string name="w"+to_string(count);
           CirGate* gate =(t0topolist[i]); 
           gate->setName(name); 
           count++;
       }
    }
 
    //write out.v
    std::cout<<"write out.v\n";     
    std::ifstream ifs1(filenamef.c_str());
    std::ofstream ofs1(filename1.c_str());
    char line[1000000];  
    
    while(ifs1.getline(line,sizeof(line),'\n')){
       if ((line[0]=='e') and (line[3]=='m')){
           for (unsigned i = 0; i < numT; ++i){
               ofs1<<"patch p"<<i<<" ("<<".t_"<<i<<"("<<"t_"<<i<<"), ";  
           }
           for (unsigned i = 0; i < pi.size(); ++i)
              if (i+1<pi.size()){
          	      ofs1 << "."<< piList_[i]->getName()<<"("<<piList_[i]->getName()<<")"<<", " ;
             }
             else {
                      ofs1<< "."<<piList_[i]->getName()<<"("<<piList_[i]->getName()<<"));"<<std::endl;
             } 
       }
       ofs1<<line<<std::endl;  
    }
    ofs1.close();
 

    //write patch.v
    std::cout<<"write patch.v\n";
    std::ofstream ofs(filename.c_str());
    
    // write module patch
  ofs <<"module patch (";        
  for (unsigned i = 0; i < numT; ++i)
    ofs<<"t_"<<i<<", ";
  for (unsigned i = 0; i < pi.size(); ++i)
    if (i+1<pi.size()){
    	ofs << piList_[i]->getName()<<", " ;
    }
    else {
  	ofs<< piList_[i]->getName()<<");"<<std::endl;
    }
  // write input
  ofs<<"input " ;
  for (unsigned i = 0; i < pi.size(); ++i)
    if (i+1<pi.size()){
    	ofs << piList_[i]->getName()<<", " ;
    }
    else {
  	ofs<< piList_[i]->getName()<<";"<<std::endl;
    }
  // write output
  ofs<<"output " ;
  for (unsigned i = 0; i < numT; ++i)
    if (i+1<numT){
    	ofs<<"t_"<<i<<", ";
    }
    else {
  	ofs<<"t_"<<i<<";"<<std::endl;
    }
  // write wire
  
  ofs<<"wire ";  
  for (unsigned i = 0; i < n-1; ++i){
   if (!t0topolist[i]->getFanin0().isNull()){
    if (i+1<n-1){
    	ofs<<t0topolist[i]->getNAME()<<", ";
    }
    else {
  	ofs<<t0topolist[i]->getNAME()<<";"<<std::endl;
    }
   }
  }
  ofs<<"\n";
  // write AIG
   for (int i=0;i<n;i++){   
       if (!t0topolist[i]->getFanin0().isNull()){
             
             CirGateV in0 = t0topolist[i]->getFanin0();
             if (!t0topolist[i]->getFanin1().isNull() ){
                  if(t0topolist[i]->getFanin1()!=t0topolist[i]->getFanin0()){
                  	CirGateV in1 = t0topolist[i]->getFanin1();   
                  	//std::cout<<"and ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" , "<<in1.gate()->getNAME()<<" )\n";
                  	ofs<<"and ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" , "<<in1.gate()->getNAME()<<" );\n";
                  }
                  else {
                         if (in0.isInv()){ 
                            //std::cout<<"not ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" )\n";
                            ofs<<"not ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" );\n";
       	                 }
                         else {  
                             ofs<<"buf ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" );\n";
                         }
                  }
             }
             else {         
                   if (in0.isInv()){ 
                   //std::cout<<"not ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" )\n";
                      ofs<<"not ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" );\n";
       	           }
                   else {  
                      ofs<<"buf ( "<<t0topolist[i]->getNAME()<<" , "<<in0.gate()->getNAME()<<" );\n";
                   }
             }
             
       }
      
       
    }    
 
  ofs<<"\n";
  // write endmodule
  ofs << "endmodule;" << std::endl;
  ofs.close();
   
}
