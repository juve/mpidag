CXX = mpic++
CFLAGS = $(shell mpic++ -g -Wall --showme:compile)
LDFLAGS = $(shell mpic++ --showme:link)
SOURCES = $(shell ls *.cpp)
TARGETS = mpidag
TESTS = $(shell ls test-*.cpp | sed 's/.cpp$$//')
OBJECTS = $(shell ls *.cpp | grep -v '^test-' | grep -v 'mpidag' | sed 's/.cpp$$/.o/')
TEST_OBJECTS = $(shell ls test-*.cpp | sed 's/.cpp$$/.o/')

.PHONY: clean depends

all: $(TARGETS)

mpidag: mpidag.o $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

test-strlib: test-strlib.o $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

test-dag: test-dag.o $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

test: $(TESTS)
	for test in $(TESTS); do echo $$test; ./$$test; done

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGETS) $(TESTS) Makefile.d

depends: Makefile.d

Makefile.d:
	g++ -MM $(SOURCES) > Makefile.d

include Makefile.d
