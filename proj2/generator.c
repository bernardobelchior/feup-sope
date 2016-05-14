/**
 * generator.c
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "vehicle.h"

#define FIFO_MODE 0777

int generate_vehicles = 1;
FILE* logger; 
int ticks;
pthread_mutex_t mutexes[4];
int fifos[4];

/**
 * Writes the vehicle status to the log file.
 */
void log_vehicle(vehicle_t *vehicle, int lifetime, vehicle_status_t v_status) {
		fprintf(logger, "%d;%d;%d;%d;%d;%s\n", ticks, vehicle->id, vehicle->direction, vehicle->parking_time, lifetime, messages_array[v_status]);
}

/* Handles the vehicle lifetime and logs everytime its state changes.
 * 
 * Returns 1 if the vehicle needs to be maintained, returning 0 otherwise.
 */
int vehicle_changed_state(vehicle_t *vehicle, int lifetime, vehicle_status_t v_status) {

	if(logger != NULL)
		log_vehicle(vehicle, lifetime, v_status);

	return v_status == ENTERED || v_status == PARK_FULL;
}

/**
 * Sends the vehicle to the specified fifo and
 * creates the vehicle specific fifo.
 */
void send_vehicle(vehicle_t* vehicle) {
	int ticks_start = ticks;

	vehicle->fifo_name = (char*) malloc(10*sizeof(char));
	sprintf(vehicle->fifo_name, "vehicle%d", vehicle->id);

	pthread_mutex_lock(&mutexes[vehicle->direction]);
	write(fifos[vehicle->direction], &(vehicle->id), sizeof(int));
	write(fifos[vehicle->direction], &(vehicle->parking_time), sizeof(int));
	write(fifos[vehicle->direction], &(vehicle->direction), sizeof(direction_t));
	write(fifos[vehicle->direction], vehicle->fifo_name, (strlen(vehicle->fifo_name)+1)*sizeof(char));
	pthread_mutex_unlock(&mutexes[vehicle->direction]);

	vehicle_status_t status = -1;
	int vehicle_fifo = open(vehicle->fifo_name, O_WRONLY);
	
	if(vehicle_fifo == -1) {
		fprintf(stderr, "The fifo named %s could not be open.\n", vehicle->fifo_name);
	} else {
		do {	
			read(vehicle_fifo, &status, sizeof(vehicle_status_t));
  		} while(vehicle_changed_state(vehicle, ticks-ticks_start, status)); 

		close(vehicle_fifo);
	}

	unlink(vehicle->fifo_name);
}

/**
 * Handles the thread for every vehicle
 */
void* vehicle_thread(void* arg) {
	vehicle_t* vehicle = (vehicle_t*) arg;

	send_vehicle(vehicle);

	return NULL;
}

/**
 * Handles the SIGALRM signal.
 */
void alarm_fired(int signo) {
	if(signo == SIGALRM)
		generate_vehicles = 0;
}

/**
 * Generates the number of ticks to the
 * next vehicle with the following probability
 * 
 * 0 ticks: 50%
 * 1 ticks: 30%
 * 2 ticks: 20%
 */
int get_ticks_to_next_vehicle() {
	int random = rand() % 10;

	if(random < 5)
		return 0;
	else if(random < 8)
		return 1;
	else
		return 2;
}

/**
 * Generates a vehicle every update_rate milliseconds
 * and initializes its thread.
 */
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

/**
 * Starts and runs the vehicle generator for generation_time seconds.
 */
void start_generator(int generation_time, int update_rate) {
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
	
	logger = fopen("gerador.log", "w"); 

	if(logger == NULL)
		fprintf(stderr, "Logger could not be open.\n");
	else
		fprintf(logger, "ticks;id;dest;t_est;t_vida;observ\n");

	if(signal(SIGALRM, alarm_fired) == SIG_ERR) {
		fprintf(stderr, "Could not set up signal handler for SIGALRM.\n");
	}

	mkfifo("fifoN", FIFO_MODE);
	mkfifo("fifoS", FIFO_MODE);
	mkfifo("fifoE", FIFO_MODE);
	mkfifo("fifoO", FIFO_MODE);
	fifos[0] = open("fifoN", O_WRONLY);
	fifos[1] = open("fifoS", O_WRONLY);
	fifos[2] = open("fifoE", O_WRONLY);
	fifos[3] = open("fifoO", O_WRONLY);
	
	int i;
	for(i = 0; i < 4; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
		if(fifos[i] == -1)
			fprintf(stderr, "The fifo %d could not be open.\n", i);
	}
	
	start_generator(generation_time, update_rate);

	for(i = 0; i < 4; i++) {
		pthread_mutex_destroy(&mutexes[i]);
		close(fifos[i]);
	}

	return 0;
}
