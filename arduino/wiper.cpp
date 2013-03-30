#include <Arduino.h>

#define PIN_WIPER 11


void wiper_init() {
	pinMode(PIN_WIPER,OUTPUT);
}

void wiper_update(int val) {
	analogWrite(PIN_WIPER,val);
}

