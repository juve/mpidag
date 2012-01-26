# Compiler options for MPICH2
CC = $(shell mpic++ -compile-info)
LD = $(shell mpic++ -link-info)
LDFLAGS = 
CFLAGS = -g -Wall
