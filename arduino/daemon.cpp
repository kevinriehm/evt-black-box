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


// Sensor state
static float latitude;
static float longitude;

static float amperage;
static float voltage;

static float mph;

// Control state
static struct {
	int head;
	struct { int l, r; } front, back;
} lights;

static int wiper;


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

	// Obey the commands of the master
	switch(com_read_cmd()) {
	case 'l':
		switch(com_read_cmd()) {
		case 'b':
			switch(com_read_cmd()) {
			case 'l': lights.back.l = com_read_int(); break;
			case 'r': lights.back.r = com_read_int(); break;

			case '\0':
			default: break;
			}
			break;

		case 'f':
			switch(com_read_cmd()) {
			case 'l': lights.front.l = com_read_int(); break;
			case 'r': lights.front.r = com_read_int(); break;

			case '\0':
			default: break;
			}

		case 'h': lights.head = com_read_int(); break;

		default: break;
		}

		analogWrite( 3,lights.head);
		analogWrite( 6,lights.front.l);
		analogWrite( 5,lights.front.r);
		analogWrite(10,lights.back.l);
		analogWrite( 9,lights.back.r);
		break;

	case 'w':
		wiper = com_read_int();
		analogWrite(11,wiper);
		break;

	case '\0':
	default: break;
	}

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
Serial2.println(millis() - sectime);
	}

	// Let's not overload this thing
	delay(1);
}

