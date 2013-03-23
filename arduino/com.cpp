/*
 * In charge of communications with the various bits of output hardware
 * (digital potentiometer, MOSFETs, serial)
 */

#include <Arduino.h>
#include <SPI.h>

#define POT_SS 10


static void pot_init();


void com_init() {
	Serial.begin(115200);

	pinMode(POT_SS,OUTPUT);

	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV128);
	SPI.setDataMode(SPI_MODE0);

	pot_init();
}

// SPI utility routines

static uint8_t spi_send8(uint8_t x) {
	return SPI.transfer(x);
}

static uint16_t spi_send16(uint16_t x) {
	uint16_t a, b;

	a = SPI.transfer(x >> 8 & 0xFF);
	b = SPI.transfer(x & 0xFF);

	return a << 8 | b;
}

// Digital potentiometer

static void pot_init() {
	digitalWrite(POT_SS,LOW);

	spi_send16(0x410F); // Ensure the pot is active
	spi_send16(0x0000); // Set wiper to zero

	digitalWrite(POT_SS,HIGH);
}

void pot_set(int val) {
	digitalWrite(POT_SS,LOW);

	spi_send16(val & 0x1FF);

	digitalWrite(POT_SS,HIGH);
}

void pot_incr() {
	digitalWrite(POT_SS,LOW);

	spi_send8(0x04);

	digitalWrite(POT_SS,HIGH);
}

void pot_decr() {
	digitalWrite(POT_SS,LOW);

	spi_send8(0x08);

	digitalWrite(POT_SS,HIGH);
}

