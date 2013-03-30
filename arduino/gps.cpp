#include <Arduino.h>

#include "TinyGPS.h"


static TinyGPS gps;


void gps_init() {
	Serial1.begin(57600);

	// Twiddle with all the settings
	Serial1.println("$PMTK101*32");
	Serial1.println("$PMTK220,200*2C");
	Serial1.println("$PMTK301,2*2E"); // Use WAAS correction
	Serial1.println("$PMTK313,1*2E"); // Enable SBAS search
	Serial1.println("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35");
}

void gps_update(float *latitude, float *longitude) {
	while(Serial1.available())
		gps.encode(Serial1.read());

	gps.f_get_position(latitude,longitude);
}

