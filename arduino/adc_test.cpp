#include <stdint.h>

#include <Arduino.h>
#include <SPI.h>


#define BPAS 35
#define LRCK 29
#define PDWN 27

#define MD0  22
#define MD1  24

#define POT  37


void setup() {
	Serial.begin(115200); // Master computer -_-

	// Silly filtering
	pinMode(BPAS,OUTPUT);
	digitalWrite(BPAS,HIGH);

	// Set mode to master at 512f_s
	pinMode(MD0,OUTPUT);
	pinMode(MD1,OUTPUT);
	digitalWrite(MD0,HIGH);
	digitalWrite(MD1,LOW);

	// Format is left at left-justified, 24-bit

	// Set up communications
	pinMode(LRCK,INPUT);

	pinMode(SS,INPUT); // Slave mode
	pinMode(SCK,INPUT);
	pinMode(MOSI,INPUT);
	pinMode(MISO,OUTPUT);

	SPCR |= _BV(SPE);
	SPCR &= ~_BV(DORD);
	SPCR &= ~_BV(MSTR);
	SPCR &= ~_BV(CPOL);
	SPCR &= ~_BV(CPHA);

	// Power up the A/D chip
	pinMode(PDWN,OUTPUT);
	digitalWrite(PDWN,HIGH);

	// This is just for a test potentiometer
	pinMode(POT,OUTPUT);
	digitalWrite(POT,HIGH);
}

inline uint8_t spi_read() {
	while(!(SPSR & _BV(SPIF)));
	return SPDR;
}

uint32_t sample() {
	volatile uint32_t bytes[3];

	// Sync with LRCK
	while(digitalRead(SS) == LOW);

	// Read in the data
	spi_read(); // Clear register
	bytes[0] = spi_read();
	bytes[1] = spi_read();
	bytes[2] = spi_read();

	// Put it all together
	return bytes[0] << 16 | bytes[1] << 8 | bytes[2];
}

void loop() {
	static uint32_t sum = 0, samples = 0;

	sum += (sample() + 0x800000) & 0xFFFFFF;
	samples++;

	if(samples >= 5) {
		Serial.println((float) 3.3*sum/samples/((uint32_t) 1 << 24),8);
		sum = 0;
		samples = 0;

	}

	delay(100);
}

