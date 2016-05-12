#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef enum { NORTH, SOUTH, EAST, WEST } direction_t;

typedef struct {
	int id;
	int parking_time;
	direction_t direction;
} vehicle_t;

int generate_vehicles = 1;

void alarm_fired(int signo) {
	if(signo == SIGALRM)
		generate_vehicles = 0;
}

void generate_vehicle(int update_rate) {
	static int nextId = 1;
	vehicle_t vehicle;

	vehicle.id = nextId;
	nextId++;
	vehicle.parking_time = (rand() % 10) + 1;
	vehicle.direction = rand() % 4;

	printf("I am vehicle with id: %d, I will park for %d ticks and use the %d exit.\n", vehicle.id, vehicle.parking_time, vehicle.direction);
}

void start_generator(int generation_time, int update_rate) {
	alarm(generation_time);

	while(generate_vehicles) {
		generate_vehicle(update_rate);
		usleep(update_rate*1000);
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
