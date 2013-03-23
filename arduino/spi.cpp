#include <SPI.h>


void spi_init() {
	pinMode(SS,OUTPUT);

	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI.setDataMode(SPI_MODE0);
}

uint8_t spi_send8(uint8_t x) {
        return SPI.transfer(x);
}

uint16_t spi_send16(uint16_t x) {
        uint16_t a, b;

        a = SPI.transfer(x >> 8 & 0xFF);
        b = SPI.transfer(x & 0xFF);

        return a << 8 | b;
}

