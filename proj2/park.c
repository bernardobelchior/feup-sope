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
#include <unistd.h>
#include <string.h>

#define NORTH 0 // configuration of the "gates"
#define WEST  1 //    0
#define SOUTH 2 // 1     3
#define EAST  3 //    2

#define NUM_CONTROLLERS 4

#define FIFO_PATH_LENGTH 8
#define FIFO_MODE 0777

void print_usage();
void *controller_func (void *arg);

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

	int n_spaces, time_open;

	n_spaces = atoi(argv[1]);
	time_open = atoi(argv[2]);

	if(n_spaces < 0 || time_open < 0){
		print_usage();
		return 1;
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
	
	for(i = 0; i < 4; i++){
		if(pthread_join(controllers[i],NULL) != 0){ 
			printf("\tmain() :: error joining controller threads\n");
			return 4;
		}
	}

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

	//TODO read requests, create valet threads and react to the closure of the park
	

	int side = (*(int *) arg);
	char fifo_path[FIFO_PATH_LENGTH];

	switch (side){
		case NORTH:
			strcpy(fifo_path,"fifoN");
			if(mkfifo(fifo_path,FIFO_MODE) != 0){
				printf("\tcontroller_func() :: failed making FIFO\n");
				exit(4);
			}
			break;

		case WEST:
			strcpy(fifo_path,"fifoW");
			if(mkfifo(fifo_path,FIFO_MODE) != 0){
				printf("\tcontroller_func() :: failed making FIFO\n");
				exit(4);
			}
			break;

		case SOUTH:
			strcpy(fifo_path,"fifoS");
			if(mkfifo(fifo_path,FIFO_MODE) != 0){
				printf("\tcontroller_func() :: failed making FIFO\n");
				exit(4);
			}
			break;

		case EAST:
			strcpy(fifo_path,"fifoE");
			if(mkfifo(fifo_path,FIFO_MODE) != 0){
				printf("\tcontroller_func() :: failed making FIFO\n");
				exit(4);
			}
			break;

		default:
			break; //not expected
	}

	if(unlink(fifo_path) != 0){
		printf("\tcontroller_func() :: failed unlinking FIFO\n");
		exit(4);
	}
	return NULL;
}
