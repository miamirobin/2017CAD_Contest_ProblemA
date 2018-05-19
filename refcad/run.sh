if [ "$#" -ne "1" ]; then
   echo "Usage: $0 <testcase index>";
   exit 0;
fi

./np3 testdata/cad16_np3_case/case$1/cir1.v testdata/cad16_np3_case/case$1/cir2.v match_$1.out
