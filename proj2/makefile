CC = gcc
CFLAGS = -Wall -lpthread -pthread
TARGET = parque
MKDIR = mkdir -p

parque: clean gerador vehicle.h
	$(CC) $(CFLAGS) park.c -o parque

gerador: vehicle.h
	$(CC) $(CFLAGS) generator.c -o gerador

clean:
	rm -f parque gerador
