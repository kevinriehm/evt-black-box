#include <Arduino.h>

void setup() {
	Serial2.begin(9600);
	
	pinMode(13,OUTPUT);
}

void loop() {
	static int led = 0;
	
	Serial2.println("{g34.7896872,42.574354;t97698716435;hf3479ab57eb2986e;}");

	digitalWrite(13,led ? HIGH : LOW);
	led = !led;

	delay(1000);
}

