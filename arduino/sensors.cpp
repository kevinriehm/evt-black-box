#include <Arduino.h>

#include "TinyGPS.h"

#define isnewline(c) ((c) == '\x0A' || (c) == '\x0D')

TinyGPS gps;

// Converts the ASCII character c from base 10 to an integer
int ctoi(int c)
{
	if('0' <= c && c <= '9')
		return c - '0';
	
	// Invalid value
	return -1;
}

void setup()
{
	Serial.begin(115200); // Computer
	Serial1.begin(57600); // GPS
}

void loop() {}

// External communication
void serialEvent()
{
	static char gpscmd[100] = "";
	static int cmd = '\0', pin = 0, passthrough = 0;
	
	float latf, lonf;
	int c = Serial.read();
	unsigned long date, time, fixage;
	
	switch(cmd) {
	// New command
	default:
	case '\0':
		cmd = c;
	break;
	
	// Query status
	case '?':
		Serial.print("OK\0");
		
		cmd = '\0';
	break;
	
	// Read pin
	case 'a':
	case 'd':
		if(isnewline(c)) { // End of command?
			// Respond
			Serial.print(cmd == 'a' ? analogRead(pin) : digitalRead(pin));
			Serial.print('\0');
			
			// Reset
			pin = 0;
			cmd = '\0';
		} else {
			// Build the pin number
			pin = 10*pin + ctoi(c);
		}
	break;
	
	// Get GPS data
	case 'g':
		// Start talking directly to the GPS device?
		if(!passthrough && c == 'p') passthrough = 1;
		else {
			// Fine, just ignore me if you want
			if(passthrough) {
				int len = strlen(gpscmd);
				gpscmd[len] = c;
				gpscmd[len + 1] = '\0';
				Serial.write(c);
			}
			
			// End of command
			if(isnewline(c)) {
				if(passthrough) {
					Serial.println();
					Serial1.print(gpscmd);
					Serial1.print("\x0D\x0A");
					gpscmd[0] = '\0';
				} else {
					gps.f_get_position(&latf,&lonf,&fixage);
					gps.get_datetime(&date,&time);
					
					Serial.println(fixage);
					
					// Spatial coordinates
					Serial.println(latf,5);
					Serial.println(lonf,5);
					Serial.println(gps.f_altitude());
					
					// Time
					Serial.println(date);
					Serial.println(time);
					
					// Vector motion
					Serial.println(gps.f_speed_mps());
					Serial.println(gps.f_course());
					
					// Miscellaneous
					Serial.println((float) gps.hdop()/100);
					Serial.println(gps.satellites());
				}
					
				Serial.print('\0');
				
				passthrough = 0;
				
				cmd = '\0';
			}
		}
	break;
	}
}

// GPS input
void serialEvent1() {
	static int output = 0, last = 0;
	
	int cur = Serial1.read();
	
	gps.encode(cur);
	
	// Echo this iff it doesnt look like a normal GPS info sentence
	if(last == '$') {
		if(cur == 'G') {
			output = 0;
		} else {
			Serial.write(last);
			output = 1;
		}
	}
	
	if(output && cur != '$')
		Serial.write(cur);
	
	last = cur;
}
