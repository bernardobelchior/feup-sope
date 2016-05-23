/**
 * generator.c
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/times.h>
#include "vehicle.h"

#define FIFO_MODE 0666
#define FIFO_NOT_CREATED 11

int generate_vehicles = 1;
int no_active_vehicles = 0;
FILE* logger; 
int ticks;
clock_t TICKS_PER_SECOND;
pthread_mutex_t mutexes[4];

/**
 * Writes the vehicle status to the log file.
 */
void log_vehicle(vehicle_t *vehicle, int lifetime, vehicle_status_t v_status) {
	if(logger != NULL) {
		//If lifetime == 0, then the lifetime is unknown, so the log
		//should display a '?'. Otherwise display the lifetime.
		char lifetime_str[10];
		if(lifetime == 0)
			sprintf(lifetime_str, "%s", "?");
		else 
			sprintf(lifetime_str, "%d", lifetime);

		fprintf(logger, "%d\t;\t%d\t;\t%s\t;\t%d\t;\t%s\t;\t%s\n", ticks, vehicle->id, direction_names[vehicle->direction], vehicle->parking_time, lifetime_str, messages_array[v_status]);
	}
}

/**
 * Handles the thread for every vehicle
 */
void* vehicle_thread(void* arg) {
	vehicle_t* vehicle = (vehicle_t*) arg;

	int ticks_start = ticks;
	char entrance_fifo[MAX_FIFONAME_SIZE];
	int entrance_fd;

	//set the name of the entrance fifo to be opened
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

	vehicle_status_t status = -1;

	if(entrance_fd == -1){
		fprintf(stderr,"Could not open fifo %s.\n", entrance_fifo);
		status = PARK_CLOSED;
		log_vehicle(vehicle, ticks-ticks_start, status);
		free(vehicle);
		close(entrance_fd);
		pthread_exit(NULL);
	}	

	sprintf(vehicle->fifo_name, "vehicle%d", vehicle->id);

	sem_t* semaphore = sem_open(semaphore_name, O_CREAT, FIFO_MODE, 1);
	if(semaphore == SEM_FAILED) {
		fprintf(stderr, "Semaphore failed to be created.\nExiting program...");
		free(vehicle);
		close(entrance_fd);
		pthread_exit(NULL);
	}

	if(mkfifo(vehicle->fifo_name, FIFO_MODE) == -1) {
		fprintf(stderr, "The fifo %s could not be created. (%s)\n", vehicle->fifo_name, strerror(errno));
		free(vehicle);
		close(entrance_fd);
		pthread_exit(NULL);
	} 

	sem_wait(semaphore);
	pthread_mutex_lock(&mutexes[vehicle->direction]);
	write(entrance_fd, &(vehicle->id), sizeof(int));
	write(entrance_fd, &(vehicle->creation_time), sizeof(int));
	write(entrance_fd, &(vehicle->parking_time), sizeof(int));
	write(entrance_fd, &(vehicle->direction), sizeof(direction_t));
	write(entrance_fd, vehicle->fifo_name, (strlen(vehicle->fifo_name)+1)*sizeof(char));
	pthread_mutex_unlock(&mutexes[vehicle->direction]);
	sem_post(semaphore);
	sem_close(semaphore);

	close(entrance_fd);
	int vehicle_fifo = open(vehicle->fifo_name, O_RDONLY);
	
	if(vehicle_fifo == -1) {
		fprintf(stderr, "The fifo named %s could not be open.\n", vehicle->fifo_name);
	} else {
		do {	
			read(vehicle_fifo, &status, sizeof(vehicle_status_t));
			printf("vehicle: %d\tstatus: %s\n", vehicle->id, messages_array[status]);
			log_vehicle(vehicle, ticks-ticks_start, status);
  		} while(status == ENTERED); 

		close(vehicle_fifo);
	}
	unlink(vehicle->fifo_name);

	free(vehicle);
	no_active_vehicles--;
	pthread_exit(0);
}

/**
 * Handles the SIGALRM signal.
 */
void alarm_fired(int signo) {
	if(signo == SIGALRM)
		//When the SIGALRM fires, the vehicle generation
		//must stop. As such, the corresponding flag is set to false.
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
	vehicle->creation_time = ticks;
	//Randomly generates a parking_time between 1 and 10
	//times the update_rate
	vehicle->parking_time = update_rate * ((rand() % 10) + 1);
	//Randomly generates a direction
	vehicle->direction = rand() % 4;

	//Creates the vehicle thread, sends the generated vehicle
	//and detaches it.
	pthread_t thread;
	pthread_create(&thread, NULL, vehicle_thread, (void *) vehicle);
	pthread_detach(thread);
}

void sleep_for_ticks(int ticks_to_sleep) {
	//struct timespec time;

	usleep(ticks_to_sleep*pow(10, 6)/TICKS_PER_SECOND);
	//usleep(ticks_to_sleep*TICKS_PER_MICROSECONDS);
}

/**
 * Starts and runs the vehicle generator for generation_time seconds.
 */
void start_generator(int generation_time, int update_rate) {
	ticks = 0;
	int ticks_to_next_vehicle = get_ticks_to_next_vehicle();
	
	//Sets up the alarm to be called in generation_time seconds
	//that will handle the vehicle generation
	alarm(generation_time);

	//Updates ticks while vehicles must be generated or 
	//while there are still active vehicles
	while(generate_vehicles || no_active_vehicles) {

		if(generate_vehicles) {
			//If the vehicle must be generated this tick, generate it
			//otherwise decrement the ticks until the next vehicle is generated
			if(ticks_to_next_vehicle == 0) {
				generate_vehicle(update_rate);
				no_active_vehicles++;
				ticks_to_next_vehicle = get_ticks_to_next_vehicle();
			} else
				ticks_to_next_vehicle--;
		}

		//Sleeps for update_rate in clock ticks
		//FIXME: Change to actual clock ticks
		sleep_for_ticks(update_rate);
		ticks += update_rate;
	}
}

int main(int argc, char* argv[]) {
	//Checks if the number of arguments is correct
	if(argc != 3) {
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\ngerador <generation_time> <update_rate>\n");
		return 1;
	}

	srand((unsigned int) time(NULL));

	int generation_time = atoi(argv[1]);
	int update_rate = atoi(argv[2]);
	TICKS_PER_SECOND = sysconf(_SC_CLK_TCK);
	
	logger = fopen("gerador.log", "w"); 

	if(logger == NULL)
		fprintf(stderr, "Logger could not be open.\n");
	else
		fprintf(logger, "ticks\t;\tid\t;\tdest\t;\tt_est\t;\tt_vida\t;\tobserv\n");

	//Sets the SIGALRM handler
	//FIXME: Change to sigaction
	if(signal(SIGALRM, alarm_fired) == SIG_ERR) {
		fprintf(stderr, "Could not set up signal handler for SIGALRM.\n");
	}

	//Initializes a mutex for every direction
	int i;
	for(i = 0; i < 4; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
	}

	start_generator(generation_time, update_rate);

	for(i = 0; i < 4; i++) {
		pthread_mutex_destroy(&mutexes[i]);
	}

	printf("Exiting generator main.\n");

	//Waits for the other thread before closing the process.
	//It is needed otherwise the process would end and close
	//all the running threads.
	pthread_exit(0);
	return 0;
}
