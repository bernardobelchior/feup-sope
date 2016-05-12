#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef enum { NORTH, SOUTH, EAST, WEST } direction;

int generate_vehicles = 1;

void alarm_fired(int signo) {
	if(signo == SIGALRM)
		generate_vehicles = 0;
}

void start_generator(int generation_time, int update_rate) {
	alarm(generation_time);

	while(generate_vehicles) {
		//generate vehicles
	}
}


int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\ngerador <generation_time> <update_rate>\n");
		return 1;
	}

	srand((unsigned int) time(NULL));

	int generation_time = atoi(argv[1]);
	int update_rate = atoi(argv[2]);

	if(signal(SIGALRM, alarm_fired) == SIG_ERR) {
		fprintf(stderr, "Could not set up signal handler for SIGALRM.\n");
	}

	start_generator(generation_time, update_rate);

	return 0;
}
