#include <Arduino.h>


#define PIN_HORN 36


void horn_init() {
	pinMode(PIN_HORN,OUTPUT);
}

void horn_update(int val) {
	digitalWrite(PIN_HORN,val ? HIGH : LOW);
}

