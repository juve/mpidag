ifndef prefix
prefix = $(HOME)
endif
bindir = $(prefix)/bin
mandir = $(prefix)/share/man
man1dir = $(prefix)/share/man/man1

CXX = mpicxx
CC = mpicxx
CXXFLAGS = -g -Wall
LDFLAGS = 
RM = rm -f
INSTALL = install
MAKE = make


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

.PHONY: clean depends test install manpages

manpages:
	cd doc && $(MAKE) manpages

install: manpages $(PROGRAMS)
	$(INSTALL) -d -m 755 $(bindir)
	$(INSTALL) -m 755 $(PROGRAMS) $(bindir)
	$(INSTALL) -d -m 755 $(man1dir)
	$(INSTALL) -m 644 doc/*.1 $(man1dir)

clean:
	$(RM) *.o $(PROGRAMS) $(TESTS)

depends:
	g++ -MM *.cpp > depends.mk

include depends.mk
