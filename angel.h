#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_thread.h>

#include "glyph-keeper/glyph.h"


#ifndef CAR
#define CAR "alpha"
#endif

#define MAIN_FONT "FreeSans.ttf"

#define SHA256_HASH_BYTES 32


typedef enum {
	DATUM_INVALID,
	DATUM_TIME,
	DATUM_POTENTIOMETER
} datum_value_id_t;

typedef struct draw_task draw_task_t;
typedef struct draw_task_funcs draw_task_funcs_t;
typedef struct draw_task_spec draw_task_spec_t;

struct draw_task_funcs {
	void (*init)(draw_task_t *task, char *args);
	void (*draw)(draw_task_t *task);
};

struct draw_task {
	SDL_Rect area;
	draw_task_funcs_t funcs;
	void *data;
	draw_task_t *next;
};

struct draw_task_spec {
	draw_task_funcs_t *funcs;
	double x, y, w, h;
	char *args;
};

#define DATUM_HEADER "Time,Potentiometer"
typedef struct {
	uint64_t time;
	uint16_t potentiometer;
	double latitude;
	double longitude;
} datum_t;


// main.c
extern void die(char *msg);
extern void print_text(char *str, int x, int y, int w, int h, int r, int g, int b);

// analog_sensor.c
extern draw_task_funcs_t analog_draw_funcs;

// clock.c
extern draw_task_funcs_t clock_draw_funcs;

// data.c
extern void data_init();
extern void data_stop();
extern void data_get(datum_t *datum);
extern datum_value_id_t data_get_id(char *value);
extern void data_get_value(void *value, datum_value_id_t id);

// draw.c
extern draw_task_funcs_t begin_draw_funcs;
extern draw_task_funcs_t finish_draw_funcs;

extern void draw_init();
extern void draw_screen();

extern SDL_Surface *screen;
extern float screenhscale, screenvscale; // Relative to 640x480

// draw_specs.c
extern const draw_task_spec_t draw_task_specs[];

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
