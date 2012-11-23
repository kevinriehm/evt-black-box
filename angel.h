#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#ifndef CAR
#define CAR "alpha"
#endif

#define MAIN_FONT "FreeSans.ttf"

#define SHA256_HASH_BYTES 32


typedef enum {
	DATUM_INVALID,
	DATUM_TIME,
	DATUM_POTENTIOMETER,
	DATUM_LATITUDE,
	DATUM_LONGITUDE
} datum_value_id_t;

#define DATUM_HEADER "Time,Potentiometer"
typedef struct {
	uint64_t time;
	uint16_t potentiometer;
	double latitude;
	double longitude;
} datum_t;


// main.c
extern void die(char *msg);

// data.c
extern void data_init();
extern void data_stop();
extern void data_get(datum_t *datum);
extern datum_value_id_t data_get_id(char *value);
extern void data_get_value(void *value, datum_value_id_t id);

// event.c
extern void event_loop();

// hmac_sha256.c
extern void hmac_sha256(uint8_t hmac[SHA256_HASH_BYTES], const void *key,
	int keysize, const void *data, int datasize);
extern void sha256_str(char *str, uint8_t hash[SHA256_HASH_BYTES]);

// log.c
extern void log_init();
extern void log_stop();

// serial.c
extern void serial_init();
extern void serial_cmd(char *result, int n, char *cmd);

