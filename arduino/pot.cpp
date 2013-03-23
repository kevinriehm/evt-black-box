#include <Arduino.h>

#include "spi.h"

#define POT_SS SS


static void enable();
static void disable();

void pot_init() {
	enable();

	spi_send16(0x410F); // Ensure the pot is active
	spi_send16(0x0000); // Set the wiper to zero

	disable();
}

void pot_set(int val) {
	enable();

	spi_send16(val & 0x1FF);

	disable();
}

static void enable() {
	digitalWrite(POT_SS,LOW);
}

static void disable() {
	digitalWrite(POT_SS,HIGH);
}

