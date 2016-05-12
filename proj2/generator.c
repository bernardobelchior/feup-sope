#include <stdio.h>
#include <stdlib.h>

typedef enum { NORTH, SOUTH, EAST, WEST } direction;

int main(int argc, char* argv[]) {
	
	if(argc != 3) {
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\ngerador <generation_time> <update_rate>\n");
		return 1;
	}

	int generation_time = atoi(argv[1]);
	int update_rate = atoi(argv[2]);

	return 0;
}
