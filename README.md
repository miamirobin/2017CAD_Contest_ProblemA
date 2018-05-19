# 2017 cad contest cada049  Problem A

Department: Design and Verification lab                  
Author: yochi                                            
last update : 01/26/2018                                 

Note: The offical website claims that the cic machine provides g++ 4.8.2 version.
      However, I found that the environment provides g++ 4.4.7 version only.
      Therefore, I use the following command:  scl enable devtoolset-6 tcsh
      Entering this command is very important.

1. How to run  
   Run cada049 directly: `   ./cada049 F.v G.v weight.txt patch.v out.v   `  
   Run all cases: ` ./com.sh `

2. How to compile  
   There is already a compiled executable file named cada049.
   Everytime you want to compile, please type `make clean` first. (important)
   And then use `make` command to compile  
