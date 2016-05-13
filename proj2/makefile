CC = gcc
CFLAGS = -Wall -lpthread
TARGET = parque
MKDIR = mkdir -p

parque: clean gerador
	$(CC) $(CFLAGS) park.c -o parque

gerador:

clean:
	rm -f parque gerador
