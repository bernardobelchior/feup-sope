CC = gcc

CFLAGS = -Wall

TARGET = rmdup

all: rmdup

rmdup: bin/rmdup.o bin/lsdir.o
	$(CC) $(CFLAGS) -o rmdup bin/rmdup.o bin/lsdir.o

bin/rmdup.o: rmdup.c lsdir.c lsdir.h
	$(CC) $(CFLAGS) -o bin/rmdup.o -c rmdup.c

bin/lsdir.o: lsdir.c lsdir.h
	$(CC) $(CFLAGS) -o bin/lsdir.o -c lsdir.c 
	
clean: $(RM) rmdup bin/*.o *~
