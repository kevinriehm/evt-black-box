#include <Arduino.h>

#include "TinyGPS.h"


static TinyGPS gps;


void gps_init() {
	Serial1.begin(57600);

		// Twiddle with all the settings
//		Serial1.println("$PMTK220,200*2C");
//		Serial1.println("$PMTK301,2*2E"); // Use WAAS correction
//		Serial1.println("$PMTK313,1*2E"); // Enable SBAS search
//		Serial1.println("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
//			"*35");
}

void gps_update(float *latitude, float *longitude, unsigned long *time) {
	static const int cum_days[] = {
		0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
	};

	int year;
	unsigned long age;
	byte month, day, hour, minute, second, centisecond;

	while(Serial1.available())
		gps.encode(Serial1.read());

	gps.f_get_position(latitude,longitude);
	gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,
		&centisecond,&age);

	*time = ((((year - 1970)*365 + cum_days[month])*24 + hour)*60
		+ minute)*60 + second + (10*centisecond + age)/1000;
}

