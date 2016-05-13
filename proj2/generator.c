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
FILE* logger; 
int ticks;

void* vehicle_thread(void* arg) {
	vehicle_t* vehicle = (vehicle_t*) arg;

	if(logger != NULL)
		fprintf(logger, "%d;%d;%d;%d\n", ticks, vehicle->id, vehicle->direction, vehicle->parking_time);

	return NULL;
}

void alarm_fired(int signo) {
	if(signo == SIGALRM)
		generate_vehicles = 0;
}

int get_ticks_to_next_vehicle() {
	int random = rand() % 10;

	if(random < 5)
		return 0;
	else if(random < 8)
		return 1;
	else
		return 2;
}

void generate_vehicle(int update_rate) {
	static int nextId = 1;
	vehicle_t* vehicle = (vehicle_t*) malloc(sizeof(vehicle_t));

	vehicle->id = nextId;
	nextId++;
	vehicle->parking_time = (rand() % 10) + 1;
	vehicle->direction = rand() % 4;

	pthread_t thread;
	pthread_create(&thread, NULL, vehicle_thread, (void*) vehicle);
	pthread_detach(thread);
}

void start_generator(int generation_time, int update_rate) {
	logger = fopen("gerador.log", "w"); 

	if(logger == NULL)
		fprintf(stderr, "Logger could not be open.\n");
	else
		fprintf(logger, "ticks;id;dest;t_est\n");
	
	ticks = 0;
	int ticks_to_next_vehicle = get_ticks_to_next_vehicle();

	alarm(generation_time);

	while(generate_vehicles) {
		if(ticks_to_next_vehicle == 0) {
			generate_vehicle(update_rate);
			ticks_to_next_vehicle = get_ticks_to_next_vehicle();
		} else
			ticks_to_next_vehicle--;

		usleep(update_rate*1000);
		ticks+= update_rate;
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
