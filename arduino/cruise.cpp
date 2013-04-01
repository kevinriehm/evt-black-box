// Simple PID-based speed regulator

#include "cruise.h"


#define WINDOW 100


static const float kp = 0.5;
static const float ki = 0.5;
static const float kd = 0.5;

static float setpoint;

static int errori;
static float errors[WINDOW];
static float errorsum;

static float samples[2];


void cruise_init() {
	for(errori = 0; errori < WINDOW; errori++)
		errors[errori] = 0;
	errori = 0;

	cruise_set(15);
}

void cruise_set(float speed) {
	setpoint = speed;
}

float cruise_calc(float sample) {
	int i;
	float e, de;

	e = sample - setpoint;

	errorsum -= errors[errori];
	errorsum += errors[errori++] = sample - setpoint;
	errori %= WINDOW;

	de = (sample - samples[1])/2;
	samples[1] = samples[0];
	samples[0] = sample;

	return kp*e + ki*errorsum + kd*e;
}

