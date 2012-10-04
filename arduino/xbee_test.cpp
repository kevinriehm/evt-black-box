#include <Arduino.h>

void setup() {
	Serial.begin(9600);
	
	pinMode(13,OUTPUT);
}

void loop() {
	static int led = 0;
	
	while(Serial.available()) {
		int c = Serial.read();
		Serial.write(c);
		
		digitalWrite(13,led ? HIGH : LOW);
		led = !led;
	}
}
