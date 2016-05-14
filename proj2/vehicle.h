/**
 * park.c
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

typedef enum { NORTH, SOUTH, EAST, WEST } direction_t;

typedef struct {
	int id;
	int parking_time;
	direction_t direction;
	char *fifo_name;
} vehicle_t;

typedef enum { PARK_FULL, ENTERED, EXITED, PARK_CLOSED} vehicle_status_t;

const char* messages_array[4] = { "cheio!", "entrada", "saida", "encerrado" };
