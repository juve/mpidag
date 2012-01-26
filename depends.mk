dag.o: dag.cpp dag.h failure.h strlib.h
failure.o: failure.cpp failure.h
log.o: log.cpp log.h
master.o: master.cpp master.h dag.h failure.h protocol.h log.h
mpidag.o: mpidag.cpp master.h dag.h worker.h failure.h log.h
protocol.o: protocol.cpp protocol.h
strlib.o: strlib.cpp strlib.h
test-dag.o: test-dag.cpp dag.h failure.h
test-log.o: test-log.cpp log.h
test-strlib.o: test-strlib.cpp strlib.h failure.h
worker.o: worker.cpp strlib.h worker.h protocol.h log.h
