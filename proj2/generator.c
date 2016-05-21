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
#include <semaphore.h>
#include <string.h>
#include "vehicle.h"

#define FIFO_MODE 0666
#define FIFO_NOT_CREATED 11

int generate_vehicles = 1;
FILE* logger; 
int ticks;
pthread_mutex_t mutexes[4];

/**
 * Writes the vehicle status to the log file.
 */
void log_vehicle(vehicle_t *vehicle, int lifetime, vehicle_status_t v_status) {
	if(logger != NULL)
		fprintf(logger, "%d;%d;%d;%d;%d;%s\n", ticks, vehicle->id, vehicle->direction, vehicle->parking_time, lifetime, messages_array[v_status]);
}

/**
 * Handles the thread for every vehicle
 */
void* vehicle_thread(void* arg) {
	vehicle_t* vehicle = (vehicle_t*) arg;

	int ticks_start = ticks;
	char entrance_fifo[MAX_FIFONAME_SIZE];
	int entrance_fd;

	switch(vehicle->direction){
		case NORTH:
			strcpy(entrance_fifo,"fifoN");
			break;

		case EAST:
			strcpy(entrance_fifo, "fifoE");
			break;

		case WEST:
			strcpy(entrance_fifo, "fifoO");
			break;

		case SOUTH:
			strcpy(entrance_fifo,"fifoS");
			break;

		default:
			break; //not expected
	}

	entrance_fd = open(entrance_fifo, O_WRONLY | O_NONBLOCK);

	if(entrance_fd == -1){
		fprintf(stderr,"Could not open fifo %s.\n", entrance_fifo);
		pthread_exit(NULL);
	}	

	sprintf(vehicle->fifo_name, "vehicle%d", vehicle->id);

	sem_t* semaphore = sem_open(semaphore_name, O_CREAT, FIFO_MODE, 1);
	if(semaphore == SEM_FAILED) {
		fprintf(stderr, "Semaphore failed to be created.\nExiting program...");
		exit(SEMAPHORE_CREATION_FAILED);
	}

	sem_wait(semaphore);
	pthread_mutex_lock(&mutexes[vehicle->direction]);
	write(entrance_fd, &(vehicle->id), sizeof(int));
	write(entrance_fd, &(vehicle->parking_time), sizeof(int));
	write(entrance_fd, &(vehicle->direction), sizeof(direction_t));
	write(entrance_fd, vehicle->fifo_name, (strlen(vehicle->fifo_name)+1)*sizeof(char));
	pthread_mutex_unlock(&mutexes[vehicle->direction]);
	sem_post(semaphore);
	sem_close(semaphore);

	if(mkfifo(vehicle->fifo_name, FIFO_MODE) == -1) {
		fprintf(stderr, "The fifo %s could not be created.\n", vehicle->fifo_name);
		free(vehicle);
		pthread_exit(NULL);
	} 

	vehicle_status_t status = -1;
	int vehicle_fifo = open(vehicle->fifo_name, O_RDONLY);
	
	if(vehicle_fifo == -1) {
		fprintf(stderr, "The fifo named %s could not be open.\n", vehicle->fifo_name);
	} else {
		do {	
			read(vehicle_fifo, &status, sizeof(vehicle_status_t));
			log_vehicle(vehicle, ticks-ticks_start, status);
  		} while(status == ENTERED); 

		close(vehicle_fifo);
	}

	unlink(vehicle->fifo_name);

	free(vehicle);
	pthread_exit(NULL);
}

/**
 * Handles the SIGALRM signal.
 */
void alarm_fired(int signo) {
	if(signo == SIGALRM)
		generate_vehicles = 0;
	printf("Alarm Fired!!\n");
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
	pthread_create(&thread, NULL, vehicle_thread, (void *) vehicle);
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

	int i;
	for(i = 0; i < 4; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
	}

	start_generator(generation_time, update_rate);

	for(i = 0; i < 4; i++) {
		pthread_mutex_destroy(&mutexes[i]);
	}

	printf("Exiting generator main\n");

	return 0;
}
