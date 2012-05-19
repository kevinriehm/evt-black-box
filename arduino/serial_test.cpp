#include <Arduino.h>


void setup()
{
	Serial.begin(9600);
}

void loop()
{
	Serial.println(millis());
	delay(500);
}
