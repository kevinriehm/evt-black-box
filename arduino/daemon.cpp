/*
 * Main code in charge of coordinating auxiliary input/output functionality.
 */

#include <Arduino.h>

#include "adc.h"
#include "com.h"
#include "gps.h"
#include "i2c.h"
#include "pot.h"
#include "speed.h"
#include "spi.h"


static float latitude;
static float longitude;

static float amperage;
static float voltage;

static float mph;


void setup() {
	amperage = 0;

	com_init();

	gps_init();

	i2c_init();

	spi_init();
	pot_init();

	speed_init();
}

void loop() {
	static unsigned long sectime = 0;

	// Get GPS data
	gps_update(&latitude,&longitude);

	if(sectime + 1000 < millis()) { // Stuff that happens at 1Hz
		sectime = millis();

		// Read the powertrain current
		amperage = amp_read();
		voltage = volt_read();

		// Read the most recent speed
		mph = speed_mph();

		// Update the cruise control setting
		pot_set(random());

		// Sync the state
		com_print("{a:%f;v:%f;g:%f,%f;s:%f;}\n",amperage,voltage,
			latitude,longitude,mph);
	}

	// Let's not overload this thing
	delay(1);
}

