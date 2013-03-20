#include <Arduino.h>


void setup()
{
	Serial.begin(9600);
	Serial2.begin(9600);

	Serial.setTimeout(5000);
}

void loop()
{
	int nread;
	char buf[20];

	Serial.print("Sent by serial 0;");

	delay(10);

	Serial.println();
	Serial.print("Received by serial 2: ");
	nread = Serial2.readBytesUntil(';',buf,20);
	Serial.write((const uint8_t *) buf,nread);
	Serial.println();

	delay(500);

	Serial2.print("Sent by serial 2;");

	delay(10);

	Serial.println();
	Serial.print("Received by serial 0: ");
	nread = Serial.readBytesUntil(';',buf,20);
	Serial.write((const uint8_t *) buf,nread);
	Serial.println();

	delay(500);
}
