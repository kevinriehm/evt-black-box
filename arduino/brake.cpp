#include <Arduino.h>

#include "lights.h"


#define PIN_BRAKE 34


void brake_init() {
	pinMode(PIN_BRAKE,INPUT);
}

void brake_update(struct lights *lights) {
	lights->brakes.l.active
		= lights->brakes.r.active
		= digitalRead(PIN_BRAKE) == HIGH;

	if(lights->back.l.power) lights->brakes.l.active = 0;
	if(lights->back.r.power) lights->brakes.r.active = 0;
}

