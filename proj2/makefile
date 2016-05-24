CC = gcc
CFLAGS = -Wall -lpthread -pthread
RM = rm
DEPS = vehicle.h
TARGET = parque

all: parque gerador

parque:
	$(CC) $(CFLAGS) park.c -o bin/parque

gerador:
	$(CC) $(CFLAGS) generator.c -o bin/gerador

clean:
	$(RM) bin/*; 
