#include <Arduino.h>

#include <stdint.h>


#define PIN 0

#define HZ     5
#define PERIOD (1000/HZ) // In milliseconds

#define MAXDATA 2048

#define LEDPIN 13


int numdata = 0;
long lastread = 0;
uint16_t data[MAXDATA];


void setup() {
	Serial.begin(115200);
	
	pinMode(LEDPIN,OUTPUT);
}

void loop() {
	static int led = 0;
	
	int now = millis();
	
	if(numdata < MAXDATA && now - PERIOD > lastread) {
		// Input a new datum and time
		data[numdata++] = analogRead(PIN);
		lastread = now - now%(PERIOD);
		
		// Blink the LED
		digitalWrite(LEDPIN,led ? HIGH : LOW);
		led = !led;
	}
}

void serialEvent() {
	int i;
	
	if(Serial.read() == 'p') {
		// Output data as CSV
		Serial.print("Index,Value (Pin "); Serial.print(PIN); Serial.print("),Voltage"); Serial.println();
		for(i = 0; i < numdata; i++) {
			Serial.print(i); Serial.print(","); Serial.print(data[i]); Serial.print(","); Serial.print((float) data[i]/1024*5); Serial.println();
		}
	}
}
