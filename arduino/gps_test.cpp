#include <Arduino.h>
#include "TinyGPS.h"

#define DATA_PREFIX_STR "$GPGGA"

void setup() {
	Serial.begin(115200);
	Serial1.begin(57600);
}

void loop() {
	static TinyGPS gps;
	
	static unsigned long nextdisplay = millis();
	
	if(Serial1.available()) {
		gps.encode(Serial1.read());
	}
	
	if(millis() > nextdisplay) {
		long lat, lon;
		
		gps.get_position(&lat,&lon);
		
		Serial.print(lat);
		Serial.print(", ");
		Serial.print(lon);
		Serial.println();
		
		nextdisplay = millis() + 1000;
	}
}
