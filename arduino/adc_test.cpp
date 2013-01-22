#include <stdint.h>

#include <Arduino.h>
#include <SPI.h>


#define VOLTAGEOFFSET 0.6
#define VOLTSPERAMP   0.006


void setup() {
	Serial.begin(115200); // Master computer -_-

	// BPAS should be HIGH
	// Mode should be master at 512f_s (MODE0=1:MODE1=0)
	// Format should be left-justified, 24-bit (FMT0=0:FMT1=0)
	// PDWN should be HIGH

	// Set up communications
	pinMode(SS,INPUT); // Slave mode
	pinMode(SCK,INPUT);
	pinMode(MOSI,INPUT);

	SPCR |= _BV(SPE);
	SPCR &= ~_BV(DORD);
	SPCR &= ~_BV(MSTR);
	SPCR &= ~_BV(CPOL);
	SPCR &= ~_BV(CPHA);
}

inline uint8_t spi_read() {
	while(!(SPSR & _BV(SPIF)));
	return SPDR;
}

uint32_t sample() {
	volatile uint32_t bytes[3];

	// Sync with LRCK
	while(digitalRead(SS) == LOW);

	// Sync with the actual data
	bytes[0] = 1;
	while(bytes[0] != 0 || bytes[1] != 0) {
		bytes[1] = bytes[0];
		bytes[0] = spi_read();
	}

	// Read in the data
	while((bytes[0] = spi_read()) == 0);
	bytes[1] = spi_read();
	bytes[2] = spi_read();

	// Put it all together
	return bytes[0] << 16 | bytes[1] << 8 | bytes[2];
}

float sample_to_volts(uint32_t s) {
	return (float) 5*s/(((uint32_t) 1 << 24) - 1);
}

void loop() {
	static float offset;
	static int nsamples = 0;
	static float samples[12];
static int num = 0;
uint32_t s = (sample() + 0x800000) & 0xFFFFFF;
Serial.print(num++);
Serial.print(", ");
Serial.print(sample_to_volts(s),8);
Serial.print(", ");
Serial.print(s >> 16 & 0xFF,16);
Serial.print(" ");
Serial.print(s >>  8 & 0xFF,16);
Serial.print(" ");
Serial.print(s >>  0 & 0xFF,16);
Serial.print(", ");
Serial.print((sample_to_volts(s) - VOLTAGEOFFSET)/VOLTSPERAMP,8);
Serial.print(", ");
Serial.print((float) 5*analogRead(0)/1023,8);
Serial.print(", ");
Serial.println(((float) 5*analogRead(0)/1023 - VOLTAGEOFFSET)/VOLTSPERAMP,8);
	int i, ngood;
	float avg, avgerror, amps, volts;
/*
	samples[nsamples++] = sample_to_volts((sample() + 0x800000) & 0xFFFFFF);

	if(millis() > 3000) {
		if(nsamples >= 10) {
			// Calculate the average
			for(avg = i = 0; i < nsamples; i++) {
				avg += samples[i];
			}
			avg /= nsamples;

			// Calculate the average error
			for(avgerror = i = 0; i < nsamples; i++) {
				avgerror += fabs(avg - samples[i]);
			}
			avgerror /= nsamples;

			// Ignore the bad ones
			for(volts = ngood = i = 0; i < nsamples; i++) {
				if(fabs(avg - samples[i]) <= avgerror) {
					volts += samples[i];
					ngood++;
				}
			}
			volts /= ngood;
Serial.println();
Serial.println(nsamples);
Serial.println((avg - VOLTAGEOFFSET)/VOLTSPERAMP,8);
Serial.println(avgerror/VOLTSPERAMP,8);
Serial.println(ngood);
			amps = (volts - VOLTAGEOFFSET)/VOLTSPERAMP;

			Serial.print(amps,8);
			Serial.print("A");
			Serial.println();

			nsamples = 0;
		}
	}
*/
/*uint8_t x[4];
while(spi_read() != 0);
while((x[0] = spi_read()) == 0);
x[1] = spi_read();
x[2] = spi_read();
x[3] = spi_read();
if(x[0] < 0x10) Serial.print(' ');
Serial.print(x[0],16);
Serial.print(' ');
if(x[1] < 0x10) Serial.print(' ');
Serial.print(x[1],16);
Serial.print(' ');
if(x[2] < 0x10) Serial.print(' ');
Serial.print(x[2],16);
Serial.print(' ');
if(x[3] < 0x10) Serial.print(' ');
Serial.print(x[3],16);
Serial.print(':');*/
	delay(100);
}

