
MPI = mpich2
#MPI = openmpi

PREFIX = $(HOME)
BINDIR = $(PREFIX)/bin

RM = rm -f
INSTALL = install

DEFAULT_CFLAGS = -g -Wall

ifeq ($(MPI),mpich)
	# the -*-info arguments sometimes give all LDFLAGS and CFLAGS, not just one or the other
	CC = $(shell mpicxx -compile-info | sed -E 's/ -W?[lL][^ ]+//g')
	LD = $(shell mpicxx -link-info | sed -E 's/ -I[^ ]+//g')
	CFLAGS = $(DEFAULT_CFLAGS)
	LDFLAGS = 
endif
ifeq ($(MPI),mpich2)
	# the -*-info arguments sometimes 	give all LDFLAGS and CFLAGS, not just one or the other
	CC = $(shell mpic++ -compile-info | sed -E 's/ -W?[lL][^ ]+//g')
	LD = $(shell mpic++ -link-info | sed -E 's/ -I[^ ]+//g')
	CFLAGS = $(DEFAULT_CFLAGS)
	LDFLAGS = 
endif
ifeq ($(MPI),openmpi)
	CC = mpic++
	LD = mpic++
	CFLAGS = $(shell mpic++ $(DEFAULT_CFLAGS) --showme:compile)
	LDFLAGS = $(shell mpic++ --showme:link)
endif

OBJS += strlib.o
OBJS += failure.o
OBJS += dag.o
OBJS += master.o
OBJS += worker.o
OBJS += protocol.o

PROGRAMS += mpidag

TESTS += test-strlib test-dag

all: $(PROGRAMS)

mpidag: mpidag.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

test-strlib: test-strlib.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

test-dag: test-dag.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

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
