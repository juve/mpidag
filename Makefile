include mpich2.mk

SOURCES = $(shell ls *.cpp)
TARGETS = mpidag
TESTS = $(shell ls test-*.cpp | sed 's/.cpp$$//')
OBJECTS = $(shell ls *.cpp | grep -v '^test-' | grep -v 'mpidag' | sed 's/.cpp$$/.o/')
TEST_OBJECTS = $(shell ls test-*.cpp | sed 's/.cpp$$/.o/')

.PHONY: clean depends

all: $(TARGETS)

mpidag: mpidag.o $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

test-strlib: test-strlib.o $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

test-dag: test-dag.o $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TESTS)
	for test in $^; do echo $$test; ./$$test; done

clean:
	rm -f mpidag.o $(OBJECTS) $(TEST_OBJECTS) $(TARGETS) $(TESTS)

depends: depends.mk

depends.mk:
	g++ -MM $(SOURCES) > depends.mk

include depends.mk
