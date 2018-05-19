cmdCommon.o: cmdCommon.cpp cmdCommon.h cmdBase.h ../util/myUsage.h
cmdInit.o: cmdInit.cpp cmdMgr.h cmdBase.h cmdCommon.h ../cir/cirCmd.h \
 ../cmd/cmdBase.h
cmdMgr.o: cmdMgr.cpp cmdMgr.h cmdBase.h ../util/myString.h
cmdParser.o: cmdParser.cpp cmdParser.h cmdMgr.h cmdBase.h
