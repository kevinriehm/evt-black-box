#include <Arduino.h>


#define TURN_TIME_TO_MPH(uS) ((10.78125*3.14159265358979323846264/12/5280)/((float) (uS)/1000000/60/60))


void setup() {
	Serial.begin(115200);
}

void loop() {
	static int state = 0;
	static float mph = 0;
	static unsigned long outputtime = 0;
	static unsigned long lastrisetime = micros();

	unsigned long time;

	// State changes
	switch(state) {
	case 0: state = digitalRead(2) == LOW ? 1 : 0; break;  // High
	case 1: state = digitalRead(2) == HIGH ? 2 : 1; break; // Low
	case 2:                                                // Rising edge
		time = micros();
		mph = TURN_TIME_TO_MPH(time - lastrisetime);
		lastrisetime = time;
		state = 0;
		break;
	}

	// Account for the wheel stopping
	time = micros();
	if(TURN_TIME_TO_MPH(time - lastrisetime) < mph)
		mph = TURN_TIME_TO_MPH(time - lastrisetime);

	if(Serial.available() && Serial.read() == '?')
		Serial.println(mph,8);
}

