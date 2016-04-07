CC = gcc

CFLAGS = -Wall

TARGET = rmdup

all: rmdup lsdir

rmdup: rmdup.c 
	$(CC) $(CFLAGS) -o rmdup rmdup.c 

lsdir: lsdir.c
	$(CC) $(CFLAGS) -o lsdir lsdir.c

clean: $(RM) rmdup bin/*.o *~
