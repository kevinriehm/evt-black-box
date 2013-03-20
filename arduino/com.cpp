/*
 * In charge of communications with the various bits of output hardware
 * (digital potentiometer, MOSFETs, serial)
 */

#include <Arduino.h>


void com_init() {
	Serial.begin(115200);
}

// Digital potentiometer via SPI
void set_pot(int cs, int val) {
	
}


