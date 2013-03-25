#include "amp.h"

#define ADC_ADDR 0x48


static uint16_t adc_read(uint8_t);
static void adc_write(uint8_t, uint16_t);

float amp_read(enum adc_source source) {
	static const uint16_t sources[] = {
		0x0000,
		0x3000
	};

	uint16_t config = 0x8000          // Trigger measurement
	                | sources[source] // Input selection
	                | 0x0E00          // Gain amplification (0 - 0.256V)
	                | 0x0100          // Single-shot mode
	                | 0x0000          // 8 samples/second
	                | 0x0000          // Traditional comparator/hysteresis
	                | 0x0000          // Comparator active low
	                | 0x0000          // Non-latching comparator
	                | 0x0003;         // Disable comparator

	
}

static uint16_t adc_read(uint8_t reg) {
	Wire.beginTransmission(ADC_ADDR);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(
}

static void adc_write(uint8_t reg, uint16_t val) {
}

