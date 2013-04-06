#include <Arduino.h>


void speed_init() {
	Serial3.begin(115200);
	Serial3.setTimeout(10);
}

float speed_mph() {
	Serial3.write('?');
	return Serial3.parseFloat()*2; // _Somebody_ said diameter instead of radius
}

