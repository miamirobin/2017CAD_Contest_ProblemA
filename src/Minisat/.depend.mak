Options.o: Options.cpp Sort.h Vec.h IntTypes.h XAlloc.h Options.h \
 ParseUtils.h
satt.o: satt.cpp ../Minisat/satt.h ../Minisat/Solver.h ../Minisat/Vec.h \
 ../Minisat/IntTypes.h ../Minisat/XAlloc.h ../Minisat/Heap.h \
 ../Minisat/IntMap.h ../Minisat/Alg.h ../Minisat/Options.h \
 ../Minisat/ParseUtils.h ../Minisat/SolverTypes.h ../Minisat/Map.h \
 ../Minisat/Alloc.h
System.o: System.cpp System.h IntTypes.h
Solver.o: Solver.cpp Alg.h Vec.h IntTypes.h XAlloc.h Sort.h System.h \
 Solver.h Heap.h IntMap.h Options.h ParseUtils.h SolverTypes.h Map.h \
 Alloc.h
