CC = gcc

CFLAGS = -Wall

TARGET = rmdup

DEPS = rmdup.h lsdir.h

all: rmdup lsdir

rmdup: rmdup.c 
	$(CC) $(CFLAGS) -o bin/rmdup rmdup.c 

lsdir: lsdir.c
	$(CC) $(CFLAGS) -o bin/lsdir lsdir.c

clean: $(RM) bin/* 
