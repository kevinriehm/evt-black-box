#define DATUM_HEADER "Time,Potentiometer,Latitude,Longitude"


typedef enum {
	DATUM_INVALID,
	DATUM_TIME,
	DATUM_POTENTIOMETER,
	DATUM_LATITUDE,
	DATUM_LONGITUDE
} datum_value_id_t;

typedef struct {
	uint64_t time;
	uint16_t potentiometer;
	double latitude;
	double longitude;
} datum_t;


extern void data_init();
extern void data_stop();

extern void data_get(datum_t *datum);
extern datum_value_id_t data_get_id(char *value);
extern void data_get_value(void *value, datum_value_id_t id);
