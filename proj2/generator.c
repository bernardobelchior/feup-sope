#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef enum { NORTH, SOUTH, EAST, WEST } direction;



void start_generator(int update_rate) {
	
}


int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\ngerador <generation_time> <update_rate>\n");
		return 1;
	}

	srand((unsigned int) time(NULL));

	int generation_time = atoi(argv[1]);
	int update_rate = atoi(argv[2]);


	start_generator(update_rate);

	return 0;
}
