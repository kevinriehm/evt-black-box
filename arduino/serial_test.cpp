#include <Arduino.h>


void setup()
{
	Serial.begin(115200);
}

void loop()
{
	Serial.println((float) millis()/1000);
	delay(500);
}
