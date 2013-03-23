/*
 * Main code in charge of coordinating auxiliary input/output functionality.
 */

#include <Arduino.h>

#include "com.h"


void setup() {
	com_init();
	pot_set(0);
}

void loop() {
	// Update the cruise control setting
	pot_set(10);

	// Let's not overload this thing
	delay(100);
}

