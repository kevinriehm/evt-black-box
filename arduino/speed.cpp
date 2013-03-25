#include <Arduino.h>


void speed_init() {
	Serial3.begin(115200);
}

float speed_mph() {
	Serial3.write('?');
	Serial3.parseFloat();
}

