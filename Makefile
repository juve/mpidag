ifndef PREFIX
PREFIX = $(HOME)
endif
BINDIR = $(PREFIX)/bin

CXX = mpicxx
CC = mpicxx
CXXFLAGS = -g -Wall
LDFLAGS = 
RM = rm -f
INSTALL = install

OBJS += strlib.o
OBJS += failure.o
OBJS += dag.o
OBJS += master.o
OBJS += worker.o
OBJS += protocol.o
OBJS += log.o

PROGRAMS += mpidag

TESTS += test-strlib
TESTS += test-dag
TESTS += test-log

all: $(PROGRAMS)

mpidag: mpidag.o $(OBJS)

test-strlib: test-strlib.o $(OBJS)
test-dag: test-dag.o $(OBJS)
test-log: test-log.o $(OBJS)

test: $(TESTS)
	for test in $^; do echo $$test; ./$$test; done

.PHONY: clean depends

install:
	$(INSTALL) -d -m 755 $(BINDIR)
	$(INSTALL) $(PROGRAMS) $(BINDIR)

clean:
	$(RM) *.o $(PROGRAMS) $(TESTS)

depends:
	g++ -MM *.cpp > depends.mk

include depends.mk
