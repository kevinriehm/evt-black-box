#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#include "glyph-keeper/glyph.h"


#define MAIN_FONT "FreeSans.ttf"

#define UPDATE_DELAY_MS 1000


typedef struct draw_task draw_task_t;
typedef struct draw_task_funcs draw_task_funcs_t;
typedef struct draw_task_spec draw_task_spec_t;

struct draw_task_funcs {
	void (*draw)(draw_task_t *task);
	void (*set_task_data)(draw_task_t *task, char *args);
};

struct draw_task {
	SDL_Rect area;
	draw_task_funcs_t *funcs;
	draw_task_t *next;
};

struct draw_task_spec {
	draw_task_funcs_t *funcs;
	double x, y, w, h;
	char *args;
};

extern SDL_Surface *screen;

// main.c
extern void die(char *msg);
extern void print_text(char *str, int x, int y, int w, int h, int r, int g, int b);

// draw.c
extern draw_task_funcs_t begin_draw_funcs;
extern draw_task_funcs_t finish_draw_funcs;

extern void init_draw();
extern void draw_screen();

// draw_specs.c
extern const draw_task_spec_t draw_task_specs[];

// clock.c
extern draw_task_funcs_t clock_draw_funcs;

// event.c
extern void event_loop();
