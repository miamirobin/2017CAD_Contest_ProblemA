SolverV.o: SolverV.cpp ../minisat/SolverV.h ../minisat/SolverTypesV.h \
 ../minisat/GlobalV.h ../minisat/VarOrderV.h ../minisat/HeapV.h \
 ../minisat/ProofV.h ../minisat/FileV.h ../minisat/SortV.h
Options.o: Options.cpp Sort.h Vec.h IntTypes.h XAlloc.h Options.h \
 ParseUtils.h
satt.o: satt.cpp ../minisat/satt.h ../minisat/Solver.h ../minisat/Vec.h \
 ../minisat/IntTypes.h ../minisat/XAlloc.h ../minisat/Heap.h \
 ../minisat/IntMap.h ../minisat/Alg.h ../minisat/Options.h \
 ../minisat/ParseUtils.h ../minisat/SolverTypes.h ../minisat/Map.h \
 ../minisat/Alloc.h
reader.o: reader.cpp reader.h
sat.o: sat.cpp ../minisat/sat.h ../minisat/SolverV.h \
 ../minisat/SolverTypesV.h ../minisat/GlobalV.h ../minisat/VarOrderV.h \
 ../minisat/HeapV.h ../minisat/ProofV.h ../minisat/FileV.h
ProofV.o: ProofV.cpp ProofV.h ../minisat/SolverTypesV.h \
 ../minisat/GlobalV.h ../minisat/FileV.h SortV.h
System.o: System.cpp System.h IntTypes.h
Solver.o: Solver.cpp Alg.h Vec.h IntTypes.h XAlloc.h Sort.h System.h \
 Solver.h Heap.h IntMap.h Options.h ParseUtils.h SolverTypes.h Map.h \
 Alloc.h
FileV.o: FileV.cpp ../minisat/FileV.h ../minisat/GlobalV.h
