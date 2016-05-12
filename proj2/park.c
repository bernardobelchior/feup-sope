#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#define NORTH 0
#define EAST  1
#define SOUTH 2
#define WEST  3

void print_usage();

int main(int argc, char *argv[]){
	
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

	pthread_t controllers[4];

	int i;

	for(i = 0; i < 4; i++){
		if(pthread_create(&controllers[i],NULL,NULL,NULL) != 0){
			printf("\tmain() :: error creating controller threads\n");
			return 3;
		}
	}
	return 0;
}

void print_usage(){

	printf("Run as: ./parque <N_LUGARES> <TEMPO_ABERTURA>\nN_LUGARES and TEMPO_ABERTURA must be positive integers\n");
}
