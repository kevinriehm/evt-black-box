/*
 * Main code in charge of coordinating auxiliary input/output functionality.
 */

#include <Arduino.h>

#include "com.h"


void setup() {
	com_init();
}

void loop() {
	// Update the cruise control setting
	set_pot(23,345);
}

