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
		if(pthread_create(&controllers[i],NULL,controller_func,NULL) != 0){ //TODO create thread functions
			printf("\tmain() :: error creating controller threads\n");
			return 3;
		}
	}

	return 0;
}

void print_usage(){

	printf("Run as: ./parque <N_LUGARES> <TEMPO_ABERTURA>\nN_LUGARES and TEMPO_ABERTURA must be positive integers\n");
}

void *controller_func(void *arg){

}
