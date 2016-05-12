CC=gcc
CFLAGS=-I 

parque: clean gerador
	$(CC) park.c -o parque -pthread

gerador: 
	$(CC) generator.c -o gerador -pthread

clean: 
	rm -f parque gerador
