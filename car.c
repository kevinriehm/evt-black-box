/*
 * High-level logic for the car
 */

#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>

#include "angel.h"
#include "aux.h"
#include "gui.h"


#define PI 3.14159265358979323846264

#define HIGH 1
#define LOW  (2.0/0xFF)
#define OFF  0


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


static FILE *logfp;

static time_t racestart;
static int waitingforstart;

static gui_obj_t *backdropobj;
static gui_obj_t *mapobj;
static gui_obj_t *needleobj;
static gui_obj_t *timerobj;
static gui_obj_t *speedobj;

static int64_t gpstime;

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

static void head_high();
static void head_low();

static void wiper_on();
static void wiper_off();

static void horn_on();
static void horn_off();

static void start_race();

static void good_exit();


void car_init() {
	// Log file
	logfp = fopen("angel.log","a");

	// GUI handlers
	gui_bind("turn_left",turn_left);
	gui_bind("turn_left_stop",turn_left_stop);
	gui_bind("turn_right",turn_right);
	gui_bind("turn_right_stop",turn_right_stop);

	gui_bind("highbeams_on",head_high);
	gui_bind("highbeams_off",head_low);

	gui_bind("wiper_on",wiper_on);
	gui_bind("wiper_off",wiper_off);

	gui_bind("horn_on",horn_on);
	gui_bind("horn_off",horn_off);

	gui_bind("start_race",start_race);

	gui_bind("exit",good_exit);

	// Find all those objects
	backdropobj = gui_find_obj("backdrop",NULL);
	mapobj = gui_find_obj("map",NULL);
	needleobj = gui_find_obj("needle",NULL);
	timerobj = gui_find_obj("timer",NULL);
	speedobj = gui_find_obj("speed",NULL);

	// Startup state

	set_light(EL_WIRE,HIGH,0);
	set_light(HEADLIGHTS,LOW,0);
	set_light(BACK_LEFT,LOW,0);
	set_light(BACK_RIGHT,LOW,0);

//	gui_set_value(mapobj,-95.38370,(1 - (log(tan(29.76429/180*PI) + 1.0/cos(29.76429/180*PI))/PI))/2,16.0);
	gui_set_value(needleobj,0.0);

	start_race();
}

void car_stop() {
	fclose(logfp);
}

// data block = '{' fields '}'
// field = fieldchar ':' fieldparameters
void car_handle_data_block(char *str) {
	double timersecs;
	enum { START, FIELD } state;

	// Parse the block
	for(state = START; str && *str; str++) {
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
			case 't': sscanf(str,"t:%"PRIi64";",&gpstime); break;
			case 'v': sscanf(str,"v:%f;",&voltage); break;

			case '}':
				if(waitingforstart) {
					if(mph > 0.1) {
						racestart = time(NULL);
						waitingforstart = 0;
					}

					timersecs = 0;
				} else timersecs = difftime(time(NULL),racestart);

				// Do stuff with the data
				gui_set_value(backdropobj,0);
/*				gui_set_value(mapobj,3,latitude,
					(1 - (log(tan(longitude)
					+ 1.0/cos(longitude))/PI))/2,16.0);*/
				gui_set_value(needleobj,1,mph);
				gui_set_value(timerobj,3,
					floor(timersecs / 3600),
					floor(fmod(timersecs,3600)/60),
					floor(fmod(timersecs,60)));
				gui_set_value(speedobj,1,mph);

				if(logfp)
					fprintf(logfp,"%"PRIi64",%f,%f,%f,%f,"
						"%f,%f",gpstime,amperage,
						voltage,latitude,longitude,
						timersecs,mph);

				state = START;
				break;

			default: break;
			}
			if(!(str = strchr(str,';')))
				return;
			break;
		}
	}
}

static void cmd(char *cmd, ...) {
	static char buf[1000];

	int nread;

	va_list ap;

	va_start(ap,cmd);

	vsprintf(buf,cmd,ap);
	nread = write(auxfd,buf,strlen(buf));
	if(nread < strlen(buf)) die("cannot write to serial port");

	va_end(ap);

	buf[0] = '\r';
	buf[1] = '\n';
	write(auxfd,buf,2);
}

static void set_light(enum lights light, float power, int blinking) {
	cmd("l%sp%i",lights[light],(int) (0xFF*power));
	cmd("l%sb%i",lights[light],blinking);
}

static void turn_left() {
	set_light(FRONT_LEFT,HIGH,1);
	set_light(BACK_LEFT,HIGH,1);
}

static void turn_left_stop() {
	set_light(FRONT_LEFT,OFF,0);
	set_light(BACK_LEFT,LOW,0);
}

static void turn_right() {
	set_light(FRONT_RIGHT,HIGH,1);
	set_light(BACK_RIGHT,HIGH,1);
}

static void turn_right_stop() {
	set_light(FRONT_RIGHT,OFF,0);
	set_light(BACK_RIGHT,LOW,0);
}

static void head_high() {
	set_light(HEADLIGHTS,HIGH,0);
}

static void head_low() {
	set_light(HEADLIGHTS,LOW,0);
}

static void wiper_on() {
	cmd("w255");
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

static void start_race() {
	waitingforstart = 1;
}

static void good_exit() {
	exit(EXIT_SUCCESS);
}

