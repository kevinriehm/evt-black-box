#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#include "glyph-keeper/glyph.h"


#define MAIN_FONT "FreeSans.ttf"

#define UPDATE_DELAY_MS 30


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


// main.c
extern void die(char *msg);
extern void print_text(char *str, int x, int y, int w, int h, int r, int g, int b);

// analog_sensor.c
extern draw_task_funcs_t analog_draw_funcs;

// data.c
extern void data_init();
extern void data_stop();

// draw.c
extern draw_task_funcs_t begin_draw_funcs;
extern draw_task_funcs_t finish_draw_funcs;

extern void draw_init();
extern void draw_screen();

extern SDL_Surface *screen;
extern float screenhscale, screenvscale; // Relative to 640x480

// draw_specs.c
extern const draw_task_spec_t draw_task_specs[];

// hmac_sha256.c
extern void hmac_sha256(uint8_t *hmac, const void *key, int keysize,
	const void *data, int datasize);
extern void sha256_str(char *str, uint8_t *hash);

// clock.c
extern draw_task_funcs_t clock_draw_funcs;

// event.c
extern void event_loop();

// serial.c
extern void serial_init();
extern void serial_cmd(char *result, int n, char *cmd);
