/*
 * Main code in charge of coordinating auxiliary input/output functionality.
 */

#include <Arduino.h>

#include "adc.h"
#include "brake.h"
#include "com.h"
#include "cruise.h"
#include "gps.h"
#include "horn.h"
#include "i2c.h"
#include "lights.h"
#include "pot.h"
#include "speed.h"
#include "spi.h"
#include "wiper.h"


// Sensor state
static unsigned long time;

static float latitude;
static float longitude;

static float amperage;
static float voltage;

static float mph;

// Control state
static struct lights lights;

static int horn;
static unsigned long hornstart;
static int wiper;


void setup() {
	latitude = longitude = 0;
	amperage = voltage = 0;
	horn = mph = 0;

	com_init();
	gps_init();
	horn_init();
	i2c_init();
	brake_init();
	lights_init(&lights);
	spi_init();
	pot_init();
	speed_init();
	cruise_init();
	wiper_init();
}

void loop() {
	static unsigned long sectime = 0;

	// Obey the commands of the master
	switch(com_read_cmd()) {
	case '?': Serial.println("OK"); break;

	case 'h':
		horn = com_read_int();
		if(horn) hornstart = millis();
		break;
	case 'l': lights_read(&lights); break;
	case 'w': wiper = com_read_int(); break;

	case '\0':
	default: break;
	}

	// Update all the outputs
	brake_update(&lights);

	horn_update(millis() - hornstart > 1000 ? 0 : horn);
	lights_update(&lights);
	wiper_update(wiper);

	// Get GPS data
	gps_update(&latitude,&longitude,&time);

	if(sectime + 1000 < millis()) { // Stuff that happens at 1Hz
		sectime = millis();

		// Read the powertrain current
		amperage = amp_read();
		voltage = volt_read();

		// Read the most recent speed
		mph = speed_mph();

		// Update the cruise control setting
		pot_set(cruise_calc(mph)*0x1FF);

		// Sync the state
		com_print("{t:%i;a:%f;v:%f;g:%f,%f;s:%f;}\n",(int) time,
			amperage,voltage,latitude,longitude,mph);
	}

	// Let's not overload this thing
	delay(1);
}

