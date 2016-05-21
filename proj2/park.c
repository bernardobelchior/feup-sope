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
#include "vehicle.h"
#include <errno.h>

#define NUM_CONTROLLERS 4

#define FIFO_MODE 0666

#define SV_IDENTIFIER -1

void print_usage();
void *controller_func (void *arg);
void *assistant_func (void *arg);

pthread_mutex_t park_mutex;
int n_vacant;

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
		print_usage();
		return 1;
	}
	
	int n_spaces, time_open, n_vacant;

	n_spaces = atoi(argv[1]);
	time_open = atoi(argv[2]);
	n_vacant = n_spaces;

	if(pthread_mutex_init(&park_mutex,NULL) != 0)
		fprintf(stderr,"park.c :: main() :: Failed to create park mutex!\n");

	if(n_spaces < 0 || time_open < 0){
		print_usage();
		return 1;
	}

	if(mkfifo("fifoN",FIFO_MODE) != 0){
		printf("Failed making fifoN\n");
	}

	if(mkfifo("fifoE",FIFO_MODE) != 0){
		printf("Failed making fifoE\n");
	}

	if(mkfifo("fifoO",FIFO_MODE) != 0){
		printf("Failed making fifoO\n");
	}

	if(mkfifo("fifoS",FIFO_MODE) != 0){
		printf("Failed making fifoS\n");
	}
	//creating the 4 "controller" threads
	pthread_t controllers[4];

	int i;
	int thrargs[NUM_CONTROLLERS];

	for(i = 0; i < NUM_CONTROLLERS; i++){
		thrargs[i] = i;
		if(pthread_create(&controllers[i],NULL,controller_func, &thrargs[i]) != 0){ 
			printf("\tmain() :: error creating controller threads\n");
			return 3;
		}
	}

	int fd_N, fd_E, fd_O, fd_S;

	fd_N = open("fifoN",O_WRONLY);
	if(fd_N == -1){
		printf("Error opening fifoN\n");
	}

	fd_E = open("fifoE",O_WRONLY);
	if(fd_E == -1){
		printf("Error opening fifoE\n");
	}

	fd_O = open("fifoO",O_WRONLY);
	if(fd_O == -1){
		printf("Error opening fifoO\n");
	}

	fd_S = open("fifoS",O_WRONLY);
	if(fd_N == -1){
		printf("Error opening fifoS\n");
	}
	
	//wait for time and send SV
	sleep(time_open);


	//sending stop vehicle
	int sv = SV_IDENTIFIER;

	if(write(fd_N, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_E, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_O, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_S, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));


	printf("\tJoining threads\n");
	for(i = 0; i < 4; i++){
		if(pthread_join(controllers[i],NULL) != 0){ 
			printf("\tmain() :: error joining controller threads\n");
		}
		printf("\tmain() :: thread %d joined\n",i);
	}
	

	printf("\tClosing fifos\n");
	close(fd_N);
	close(fd_E);
	close(fd_O);
	close(fd_S);

	printf("\tunlinking fifos\n");
	unlink("fifoN");
	unlink("fifoS");
	unlink("fifoE");
	unlink("fifoO");


	printf("Exiting main...\n");

	return 0;
}

/**
 * text to be printed when the program is used incorrectly
 * 
 */
void print_usage(){

	printf("Run as: ./parque <N_LUGARES> <TEMPO_ABERTURA>\nN_LUGARES and TEMPO_ABERTURA must be positive integers\n");
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

	//TODO create valet threads 
	

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
		printf("\tError opening file\n");
	}

	//Read the request
	//Each vehicle the generator creates will have the following info
	// - vehicle identifier
	// - parking time (in clock ticks)
	// - entrance
	// - name of its private fifo
	
	int curr_entrance, curr_park_time, curr_id = 0;
	char curr_fifoname[MAX_FIFONAME_SIZE];	

	int x;
	while(curr_id  != SV_IDENTIFIER) {
		x = read(fifo_fd,&curr_id,sizeof(int));

		if(curr_id == SV_IDENTIFIER){
			printf("Closing park..\n");
			break;
		}

		read(fifo_fd,&curr_park_time,sizeof(int));
		read(fifo_fd,&curr_entrance,sizeof(int));
		read(fifo_fd,curr_fifoname,MAX_FIFONAME_SIZE);

		if(x > 0 && curr_id != SV_IDENTIFIER)
			printf("\n-----------\nThis is entrance %d\n\nid: %d\ntime: %d\nentrance: %d\nname: %s\n",
				side,curr_id, curr_park_time, curr_entrance,curr_fifoname);

		
		pthread_t assistant_tid;
		vehicle_t vehicle;

		vehicle.id = curr_id;
		vehicle.parking_time = curr_park_time;
		vehicle.direction = curr_entrance;
		strcpy(vehicle.fifo_name,curr_fifoname);

		pthread_create(&assistant_tid,NULL,assistant_func,&vehicle);
		pthread_detach(assistant_tid);
	}

	printf("Closing park..\n");
	//park now closed, handle all remaining requests
	
	while(read(fifo_fd, &curr_id,sizeof(int)) > 0){
		printf("here\n");
	}

	printf("Closing fifos\n");

	printf("\tExiting controller thread\n");
	pthread_exit(NULL);
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
	int id = (*(vehicle_t *)arg).id;	
	int park_time = (*(vehicle_t *)arg).parking_time;	
	char fifo_name[MAX_FIFONAME_SIZE];
	int fifo_fd;
	vehicle_status_t status;

	strcpy(fifo_name, (*(vehicle_t *)arg).fifo_name);

	fifo_fd = open(fifo_name, O_WRONLY);
	if( fifo_fd == -1){
		fprintf(stderr,"The fifo %s could not be opened.\n",fifo_name);
	}

	//check for vacant parking spots
	
	pthread_mutex_lock(&park_mutex);
	if(n_vacant == 0){ //no parking spots available, sends a message and exits
		status = PARK_FULL;
		write(fifo_fd,&status,sizeof(vehicle_status_t));
		pthread_exit(NULL);
	}

	else { //there are spots available
		n_vacant--;
		status = ENTERED;
		write(fifo_fd,&status,sizeof(vehicle_status_t));
	}
	pthread_mutex_unlock(&park_mutex);

	sleep(park_time);

	n_vacant--;
	status = EXITED; 
	write(fifo_fd,&status,sizeof(vehicle_status_t));

	pthread_exit(NULL);

}
