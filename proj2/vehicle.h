typedef enum { NORTH, SOUTH, EAST, WEST } direction_t;

typedef struct {
	int id;
	int parking_time;
	direction_t direction;
} vehicle_t;
