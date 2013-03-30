/*
 * High-level logic for the car
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>

#include "gui.h"


enum lights {
	EL_WIRE,
	HEADLIGHTS,
	FRONT_LEFT,
	FRONT_RIGHT,
	BACK_LEFT,
	BACK_RIGHT
};

static const char *lights[] = {
	[EL_WIRE]     = "e",
	[HEADLIGHTS]  = "h",
	[FRONT_LEFT]  = "fl",
	[FRONT_RIGHT] = "fr",
	[BACK_LEFT]   = "bl",
	[BACK_RIGHT]  = "br"
};

static int auxfd;
static FILE *auxfp;

static gui_obj_t *needleobj;

static double latitude;
static double longitude;

static double amperage;
static double voltage;

static double mph;


static void set_light(enum lights, float, int);

static void turn_left();
static void turn_left_stop();
static void turn_right();
static void turn_right_stop();

static void wiper_on();
static void wiper_off();

static void horn_on();
static void horn_off();


void car_init() {
	auxfd = open("/dev/ttyACM0",O_RDWR);
	auxfp = fdopen(auxfd,"r+");
auxfp = stdout;
	// GUI handlers
	gui_bind("turn_left",turn_left);
	gui_bind("turn_left_stop",turn_left_stop);
	gui_bind("turn_right",turn_right);
	gui_bind("turn_right_stop",turn_right_stop);

	gui_bind("wiper_on",wiper_on);
	gui_bind("wiper_off",wiper_off);

	gui_bind("horn_on",horn_on);
	gui_bind("horn_off",horn_off);

	// Find all those objects
	needleobj = gui_find_obj("needle",NULL);

	// Startup state
	set_light(EL_WIRE,1.0,0);
	set_light(HEADLIGHTS,0.5,0);
	gui_set_value(needleobj,14.2);
}

void car_stop() {
}

static void cmd(char *cmd, ...) {
	va_list ap;

	va_start(ap,cmd);

	vfprintf(auxfp,cmd,ap);

	va_end(ap);

	fputc('\n',auxfp);
}

static void set_light(enum lights light, float power, int blinking) {
	cmd("l%sp%i",lights[light],(int) (0xFF*power));
	cmd("l%sb%i",lights[light],blinking);
}

static void turn_left() {
	set_light(FRONT_LEFT,0.5,1);
	set_light(BACK_LEFT,0.5,1);
}

static void turn_left_stop() {
	set_light(FRONT_LEFT,0.5,0);
	set_light(BACK_LEFT,0.5,0);
}

static void turn_right() {
	set_light(FRONT_RIGHT,0.5,1);
	set_light(BACK_RIGHT,0.5,1);
}

static void turn_right_stop() {
	set_light(FRONT_RIGHT,0.5,0);
	set_light(BACK_RIGHT,0.5,0);
}

static void wiper_on() {
	cmd("w200");
}

static void wiper_off() {
	cmd("w0");
}

static void horn_on() {
	cmd("h1");
}

static void horn_off() {
	cmd("h0");
}

