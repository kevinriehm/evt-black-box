/*
 * Main code in charge of coordinating auxiliary input/output functionality.
 */

#include <Arduino.h>

#include "com.h"
#include "pot.h"
#include "spi.h"


static float amperage;


void setup() {
	amperage = 0;

	com_init();

	i2c_init();

	spi_init();
	pot_init();
}

void loop() {
	// Read the powertrain current
	amperage = amp_read(ADC_DIFF_01);

	// Update the cruise control setting
	pot_set(10);

	// Sync the state
	com_print("{a:%f;}\n",amperage);

	// Let's not overload this thing
	delay(100);
}

