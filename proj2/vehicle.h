/**
 * vehicle.h
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

#define MAX_FIFONAME_SIZE 32
#define FIFO_MODE 0777

typedef enum { NORTH, SOUTH, EAST, WEST } direction_t;

const char* direction_names[4] = { "North", "South", "East", "West"};

typedef struct {
	int id;
	int creation_time;
	int parking_time;
	direction_t direction;
	char fifo_name[MAX_FIFONAME_SIZE];
} vehicle_t;

typedef enum { PARK_FULL, ENTERED, EXITED, PARK_CLOSED} vehicle_status_t;

const char* messages_array[4] = { "cheio!", "entrada", "saida", "encerrado" };
#define semaphore_name "/semaphore"
