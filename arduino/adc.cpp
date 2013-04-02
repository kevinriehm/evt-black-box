/*
 * Handles reading the ammeter and voltmeter attached to the ADS1115 A/D chip.
 */

#include <Arduino.h>
#include <Wire.h>

#include "adc.h"

#define ADC_ADDR 0x48

#define CONV_REG 0x00
#define CNFG_REG 0x01

#define AMP_SRC  ADC_01
#define VLT_SRC  ADC_23


enum adc_source { ADC_01, ADC_23 };

static const uint16_t sources[] = {
	[ADC_01] = 0x0000,
	[ADC_23] = 0x3000
};

static const uint16_t configbase =
	  0x8000  // Trigger measurement
	          // Input selection
//	| 0x0E00  // Gain amplification (0 - 0.256V)
	| 0x0100  // Single-shot mode
	| 0x00E0  // 8 samples/second
	| 0x0000  // Traditional comparator/hysteresis
	| 0x0000  // Comparator active low
	| 0x0000  // Non-latching comparator
	| 0x0003; // Disable comparator


static uint16_t adc_read(uint8_t);
static void adc_write(uint8_t, uint16_t);


float amp_read() {
	adc_write(CNFG_REG,configbase | sources[AMP_SRC]);
	while(!(adc_read(CNFG_REG) & 0x8000));
	return (float) adc_read(CONV_REG)/0x7FFF*6.144/0.010;
}

float volt_read() {
	adc_write(CNFG_REG,configbase | sources[VLT_SRC]);
	while(!(adc_read(CNFG_REG) & 0x8000));
	return (float) adc_read(CONV_REG)/0x7FFF*6.144/0.0745910781;
}

static uint16_t adc_read(uint8_t reg) {
	uint8_t a, b;

	Wire.beginTransmission(ADC_ADDR);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(ADC_ADDR,2);
	a = Wire.read();
	b = Wire.read();

	return a << 8 | b;
}

static void adc_write(uint8_t reg, uint16_t val) {
	Wire.beginTransmission(ADC_ADDR);
	Wire.write(reg);
	Wire.write(val >> 8 & 0xFF);
	Wire.write(val & 0xFF);
	Wire.endTransmission();
}

