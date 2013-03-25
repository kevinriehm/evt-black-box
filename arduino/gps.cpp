#include <Arduino.h>

#include "TinyGPS.h"


static TinyGPS gps;


void gps_init() {
	Serial1.begin(57600);
}

void gps_update(float *latitude, float *longitude) {
	while(Serial1.available())
		gps.encode(Serial1.read());

	gps.f_get_position(latitude,longitude);
}

