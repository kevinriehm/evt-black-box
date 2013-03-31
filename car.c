/*
 * High-level logic for the car
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

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

static float latitude;
static float longitude;

static float amperage;
static float voltage;

static float mph;


static void set_light(enum lights, float, int);

static void turn_left();
static void turn_left_stop();
static void turn_right();
static void turn_right_stop();

static void wiper_on();
static void wiper_off();

static void horn_on();
static void horn_off();

static void good_exit();


void car_init() {
	auxfd = open("/dev/ttyACM0",O_RDWR);
	auxfp = fdopen(auxfd,"w");
//auxfp = stdout;
	// GUI handlers
	gui_bind("turn_left",turn_left);
	gui_bind("turn_left_stop",turn_left_stop);
	gui_bind("turn_right",turn_right);
	gui_bind("turn_right_stop",turn_right_stop);

	gui_bind("wiper_on",wiper_on);
	gui_bind("wiper_off",wiper_off);

	gui_bind("horn_on",horn_on);
	gui_bind("horn_off",horn_off);

	gui_bind("exit",good_exit);

	// Find all those objects
	needleobj = gui_find_obj("needle",NULL);

	// Startup state
	set_light(EL_WIRE,1.0,0);
	set_light(HEADLIGHTS,0.5,0);
}

void car_stop() {
}

// data block = '{' fields '}'
// field = fieldchar ':' fieldparameters
void car_handle_data_block(char *str) {
	enum {
		START,
		FIELD
	} state;

	// Parse the block
	for(; str && *str; str++) {
		switch(state) {
		case START:
			if(*str == '{') state = FIELD;
			break;

		case FIELD:
			switch(*str) {
			case 'a': sscanf(str,"a:%f;",&amperage); break;
			case 'g': sscanf(str,"g:%f,%f;",&latitude,&longitude);
				break;
			case 's': sscanf(str,"s:%f;",&mph); break;
			case 'v': sscanf(str,"v:%f;",&voltage); break;
			case '}': break;
			}
			str = strchr(str,';');
			break;
		}
	}

	// Do stuff with the data
	gui_set_value(needleobj,mph);
}

static void cmd(char *cmd, ...) {
	static char buf[1000];

	va_list ap;

	va_start(ap,cmd);

	vsprintf(buf,cmd,ap);
	write(auxfd,buf,strlen(buf));
	vprintf(cmd,ap);

	va_end(ap);

	buf[0] = '\n';
	write(auxfd,buf,1);
	putchar('\n');
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

static void good_exit() {
	exit(EXIT_SUCCESS);
}

