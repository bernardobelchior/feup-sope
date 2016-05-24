/**
 * park.c
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "vehicle.h"
#include <errno.h>
#include <math.h>

#define NUM_CONTROLLERS 4
#define SV_IDENTIFIER -1

pthread_mutex_t park_mutex;
int n_vacant, closed, n_spaces;
FILE *logger;
clock_t TICKS_PER_SECOND;


/**
 * vehicle logger
 *
 * puts information about a vehicle in a CSV file
 * */
void log_vehicle(int tick,int vehicle_id, vehicle_status_t status){
	fprintf(logger,"%d\t;\t%d\t;\t%d\t;\t%s\n",tick,n_spaces - n_vacant, vehicle_id, messages_array[status]);
}

/**
 * Sleeps for ticks_to_sleep measured in clock ticks
 */
void sleep_for_ticks(int ticks_to_sleep){
	struct timespec time_to_sleep;
	struct timespec time_remaining;

	time_to_sleep.tv_sec = ticks_to_sleep/TICKS_PER_SECOND;
	time_to_sleep.tv_nsec = (long) (ticks_to_sleep*pow(10, 9)/TICKS_PER_SECOND) % (long) pow(10,9);
	while(nanosleep(&time_to_sleep, &time_remaining)){
		if(errno == EINTR){
			time_to_sleep.tv_sec = time_remaining.tv_sec;
			time_to_sleep.tv_nsec = time_remaining.tv_nsec;
		}
		else
			fprintf(stderr,"park.c :: sleep_for_ticks() :: Error with nanosleep (%s)\n",strerror(errno));
	}
}

/**
 * vehicle assistant function, should:
 * 	get information about the vehicle
 * 	check if there is a parking spot for the vehicle
 * 		if there is, make a reservation and send a confirmation message through the private vehicle fifo
 * 		if there is not, send a failure message through the private vehicle fifo
 * 	if a vehicle enters the park sucessfully, control the time it stays and send an exit message through the fifo afterwards
 * 	after the exit, update the number of available spots
 */
void *assistant_func(void *arg){
	
	//get vehicle info
	int id = (*(vehicle_t *) arg).id;
	int tick_created = (*(vehicle_t *)arg).creation_time;
	int park_time = (*(vehicle_t *)arg).parking_time;	
	char fifo_name[MAX_FIFONAME_SIZE];
	int fifo_fd;
	vehicle_status_t status;

	strcpy(fifo_name, (*(vehicle_t *)arg).fifo_name);

	fifo_fd = open(fifo_name, O_WRONLY);
	if(fifo_fd == -1){
		fprintf(stderr,"park.c :: assistant thread :: the fifo %s could not be opened. (%s)\n", fifo_name, strerror(errno));
	}

	//check for vacant parking spots, using a mutex to avoid racing condition
	pthread_mutex_lock(&park_mutex);

	if(closed){
		status = PARK_CLOSED;
	}
	else if(n_vacant == 0){ //no parking spots available
		status = PARK_FULL;
	}
	else { //there are spots available
		n_vacant--;
		status = ENTERED;
	}

	pthread_mutex_unlock(&park_mutex);


	write(fifo_fd, &status, sizeof(vehicle_status_t));
	log_vehicle(tick_created, id, status);
	
	if(status != ENTERED){ //discard the thread, vehicle did not enter
		close(fifo_fd);
		pthread_exit(NULL);
	}

	//wait for the vehicle park time to end
	sleep_for_ticks(park_time);

	//removes the vehicle from the park and sends the appropriate message
	n_vacant++;
	status = EXITED; 
	write(fifo_fd,&status,sizeof(vehicle_status_t));
	log_vehicle(tick_created + park_time,id,status);
	close(fifo_fd);

	pthread_exit(NULL);
}

/**
 * controller thread:
 *
 * should create a FIFO,
 * handle its requests (entrance, time for parking, vehicle identifier, name of private FIFO for vehicle thread)
 * create an employee thread to handle the request and pass it its info
 * wait for closure of the park to attend any remaining requests and close the FIFO
 */
void *controller_func(void *arg){

	//Creating FIFO
	direction_t side = (*(int *) arg);
	char fifo_path[MAX_FIFONAME_SIZE];

	switch (side){
		case NORTH:
			strcpy(fifo_path,"fifoN");
			break;

		case WEST:
			strcpy(fifo_path,"fifoO");
			break;

		case SOUTH:
			strcpy(fifo_path,"fifoS");
			break;

		case EAST:
			strcpy(fifo_path,"fifoE");
			break;

		default:
			break; //not expected
	}

	//opening the fifo for reading
	int fifo_fd;
	fifo_fd = open(fifo_path,O_RDONLY);

	if(fifo_fd == -1){
		fprintf(stderr,"park.c :: controller thread :: Error opening fifo\n");
	}

	//Read the request
	//Each vehicle the generator creates will have the following info
	// - vehicle identifier
	// - parking time (in clock ticks)
	// - entrance
	// - name of its private fifo
	
	int curr_entrance, curr_park_time, curr_id = 0, tick_created;
	char curr_fifoname[MAX_FIFONAME_SIZE];	

	int x;
	while(curr_id  != SV_IDENTIFIER) {
		x = read(fifo_fd,&curr_id,sizeof(int));

		if(curr_id == SV_IDENTIFIER){ 
			closed = 1;
			break;
		}

		read(fifo_fd,&tick_created,sizeof(int));
		read(fifo_fd,&curr_park_time,sizeof(int));
		read(fifo_fd,&curr_entrance,sizeof(int));
		read(fifo_fd,curr_fifoname,MAX_FIFONAME_SIZE);

		//creating an assistant thread and a vehicle struct to send the info about the vehicle
		pthread_t assistant_tid;
		vehicle_t vehicle;

		vehicle.id = curr_id;
		vehicle.creation_time = tick_created;
		vehicle.parking_time = curr_park_time;
		vehicle.direction = curr_entrance;
		strcpy(vehicle.fifo_name,curr_fifoname);

		pthread_create(&assistant_tid,NULL,assistant_func,&vehicle);
		pthread_detach(assistant_tid);
	}

	
	//park now closed, handle all remaining requests
	while(read(fifo_fd, &curr_id,sizeof(int)) > 0){
		read(fifo_fd,&tick_created,sizeof(int));
		read(fifo_fd,&curr_park_time,sizeof(int));
		read(fifo_fd,&curr_entrance,sizeof(int));
		read(fifo_fd,curr_fifoname,MAX_FIFONAME_SIZE);
		
		pthread_t assistant_tid;
		vehicle_t vehicle;

		vehicle.id = curr_id;
		vehicle.creation_time = tick_created;
		vehicle.parking_time = curr_park_time;
		vehicle.direction = curr_entrance;
		strcpy(vehicle.fifo_name, curr_fifoname);

		//creating an assistant thread and a vehicle struct to send the info about the vehicle
		pthread_create(&assistant_tid,NULL,assistant_func,&vehicle);
		pthread_detach(assistant_tid);
	}

	//close the entrance fifo and exit
	close(fifo_fd);

	pthread_exit(0);
}

/**
 * main thread:
 * 
 * should initialize the needed global variables, 
 * create 4 controller threads and wait for their termination,
 * calculate and save global statistics
 *
 */
int main(int argc, char *argv[]){
	
	//Validating input
	if(argc != 3){
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\n./parque <number_parking_spots> <working_time>\n");
		return 1;
	}
	
	int time_open;

	n_spaces = atoi(argv[1]);
	time_open = atoi(argv[2]);
	
	if(n_spaces < 0 || time_open < 0){
		fprintf(stderr, "Incorrect use of program.\nShould be used in this format:\n./parque <number_parking_spots> <working_time>\nWhere <number_parking_spots> and <working_time> must be positive integers.\n");
		return 1;
	}

	n_vacant = n_spaces;
	closed = 0;
	TICKS_PER_SECOND = sysconf(_SC_CLK_TCK);

	logger = fopen("parque.log","w");
	if(logger == NULL){
		fprintf(stderr,"park.c :: main() :: Failed to open parque.log");
	}

	fprintf(logger,"t(ticks);\tnlug\t;\tid_viat\t;\tobserv\n");

	if(pthread_mutex_init(&park_mutex,NULL) != 0)
		fprintf(stderr,"park.c :: main() :: Failed to create park mutex!\n");

	if(mkfifo("fifoN",FIFO_MODE) != 0){
		fprintf(stderr,"park.c :: main() :: Failed making fifoN\n");
	}

	if(mkfifo("fifoE",FIFO_MODE) != 0){
		fprintf(stderr, "park.c :: main() :: Failed making fifoE\n");
	}

	if(mkfifo("fifoO",FIFO_MODE) != 0){
		fprintf(stderr,"park.c :: main() :: Failed making fifoO\n");
	}

	if(mkfifo("fifoS",FIFO_MODE) != 0){
		fprintf(stderr,"park.c :: main() :: Failed making fifoS\n");
	}
	
	//creating the 4 "controller" threads
	pthread_t controllers[4];

	int i;
	int thrargs[NUM_CONTROLLERS];

	for(i = 0; i < NUM_CONTROLLERS; i++){
		thrargs[i] = i;
		if(pthread_create(&controllers[i],NULL,controller_func, &thrargs[i]) != 0){ 
			printf("park.c :: main() :: error creating controller threads\n");
			return 3;
		}
	}

	int fd_N, fd_E, fd_O, fd_S;

	fd_N = open("fifoN",O_WRONLY);
	if(fd_N == -1){
		fprintf(stderr,"park.c :: main() :: Error opening fifoN\n");
	}

	fd_E = open("fifoE",O_WRONLY);
	if(fd_E == -1){
		fprintf(stderr,"park.c :: main() :: Error opening fifoE\n");
	}

	fd_O = open("fifoO",O_WRONLY);
	if(fd_O == -1){
		fprintf(stderr,"park.c :: main() :: Error opening fifoO\n");
	}

	fd_S = open("fifoS",O_WRONLY);
	if(fd_N == -1){
		fprintf(stderr,"park.c :: main() :: Error opening fifoS\n");
	}
	
	//wait for time and send SV
	sleep(time_open);

	//opening a semaphore to stop the generator from sending vehicles to write the SV
	sem_t *semaphore = sem_open(semaphore_name, O_CREAT, FIFO_MODE, 1);
	if(semaphore == SEM_FAILED) {
		fprintf(stderr,"park.c :: main() :: semaphore failed to be created.\nExiting program...");
		pthread_exit(NULL);
	}
	     
	sem_wait(semaphore);

	int sv = SV_IDENTIFIER;

	if(write(fd_N, &sv, sizeof(int)) == -1)
		fprintf(stderr,"park.c :: main() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_E, &sv, sizeof(int)) == -1)
		fprintf(stderr,"park.c :: main() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_O, &sv, sizeof(int)) == -1)
		fprintf(stderr,"park.c :: main() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_S, &sv, sizeof(int)) == -1)
		fprintf(stderr,"park.c :: main() :: error writing sv - %s\n",strerror(errno));

	sem_post(semaphore);
	sem_close(semaphore);
	sem_unlink(semaphore_name);

	//Closing fifos, joining the controller threads and unlinking entrance fifos
	close(fd_N);
	close(fd_E);
	close(fd_O);
	close(fd_S);

	for(i = 0; i < 4; i++){
		if(pthread_join(controllers[i],NULL) != 0){ 
			fprintf(stderr,"park.c :: main() :: error joining controller threads\n");
		}
	}

	if(unlink("fifoN") != 0)
		fprintf(stderr,"park.c :: main() :: failed unlinking fifoN\n");

	if(unlink("fifoE") != 0)
		fprintf(stderr,"park.c :: main() :: failed unlinking fifoE\n");

	if(unlink("fifoO") != 0)
		fprintf(stderr,"park.c :: main() :: failed unlinking fifoO\n");
	
	if(unlink("fifoS") != 0)
		fprintf(stderr,"park.c :: main() :: failed unlinking fifoS\n");

	pthread_exit(0);
}

