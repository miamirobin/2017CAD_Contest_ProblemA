reader.o: reader.cpp reader.h
ProofV.o: ProofV.cpp ProofV.h ../minisat/SolverTypesV.h \
 ../minisat/GlobalV.h ../minisat/FileV.h SortV.h
sat.o: sat.cpp ../minisat/sat.h ../minisat/SolverV.h \
 ../minisat/SolverTypesV.h ../minisat/GlobalV.h ../minisat/VarOrderV.h \
 ../minisat/HeapV.h ../minisat/ProofV.h ../minisat/FileV.h
SolverV.o: SolverV.cpp ../minisat/SolverV.h ../minisat/SolverTypesV.h \
 ../minisat/GlobalV.h ../minisat/VarOrderV.h ../minisat/HeapV.h \
 ../minisat/ProofV.h ../minisat/FileV.h ../minisat/SortV.h
FileV.o: FileV.cpp ../minisat/FileV.h ../minisat/GlobalV.h
