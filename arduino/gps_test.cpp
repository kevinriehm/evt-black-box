#include <Arduino.h>

#define DATA_PREFIX_STR "$GPGGA"

void setup() {
	Serial.begin(115200);
	Serial1.begin(57600);
}

void loop() {
	static char gpsstr[256] = "";
	static unsigned long nextdisplay = millis();
	
	static float lat, lon;
	static char lathemi, lonhemi;
	
	if(Serial1.available()) {
		char c = Serial1.read();
		int gpsstrlen = strlen(gpsstr);
		
		gpsstr[gpsstrlen] = c;
		gpsstr[gpsstrlen + 1] = '\0';
		
		if(c == 0x0A) {
			if(strncmp(gpsstr,DATA_PREFIX_STR,strlen(DATA_PREFIX_STR)) == 0) {
				if(millis() > nextdisplay) {
					Serial.print(gpsstr);
					nextdisplay = millis() + 1000;
				}
				sscanf(gpsstr,DATA_PREFIX_STR ",%f,%c,%f,%c",&lat,&lathemi,&lon,&lonhemi);
			}
			gpsstr[0] = '\0';
		}
		
		if(millis() > nextdisplay) {
			char infostr[256];
			
			/*Serial.print(lat);
			Serial.print(lathemi);
			Serial.print(", ");
			Serial.print(lon);
			Serial.print(lonhemi);
			Serial.println();
			nextdisplay = millis() + 1000;*/
		}
	}
}
