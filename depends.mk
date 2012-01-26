dag.o: dag.cpp dag.h failure.h strlib.h
failure.o: failure.cpp failure.h
master.o: master.cpp master.h dag.h failure.h protocol.h
mpidag.o: mpidag.cpp master.h dag.h worker.h failure.h
protocol.o: protocol.cpp protocol.h
strlib.o: strlib.cpp strlib.h
test-dag.o: test-dag.cpp dag.h failure.h
test-strlib.o: test-strlib.cpp strlib.h failure.h
worker.o: worker.cpp strlib.h worker.h protocol.h
