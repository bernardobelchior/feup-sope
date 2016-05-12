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

#define NORTH 0 //     0
#define WEST  1 //  1     3
#define SOUTH 2 //     2
#define EAST  3

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

	for(i = 0; i < 4; i++){
		if(pthread_create(&controllers[i],NULL,controller_func,NULL) != 0){ 
			printf("\tmain() :: error creating controller threads\n");
			return 3;
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

	//TODO create FIFOS, read requests, create valet threads and react to the closure of the park
}
