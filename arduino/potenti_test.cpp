#include <Arduino.h>

void setup()
{
	Serial.begin(115200);
}

void loop()
{
	if(Serial.available())
	{
		switch(Serial.read())
		{
			case '0': Serial.println(analogRead(0)); break;
		}
	}
}
