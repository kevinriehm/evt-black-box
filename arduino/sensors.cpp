#include <Arduino.h>


// Converts the ASCII character c from base 2-36 to an integer
int ctoi(int c)
{
	if('0' <= c && c <= '9')
		return c - '0';
	
	c = tolower(c);
	if('a' <= c && c <= 'z')
		return c - 'a';
	
	// Invalid value
	return -1;
}

void setup()
{
	Serial.begin(115200);
}

void loop() {}

void serialEvent()
{
	int cmd = Serial.read(), pin;
	
	if(cmd == '?') // Query status
		Serial.print("OK\0");
	else if(cmd == 'a' || cmd == 'd') // Read pin
	{
		// Get the pin number
		while(!Serial.available());
		pin = ctoi(Serial.read());
		
		// Respond
		Serial.print(cmd == 'a' ? analogRead(pin) : digitalRead(pin));
		Serial.print('\0');
	}
}
