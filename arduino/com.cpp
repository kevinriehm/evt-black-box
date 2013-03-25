#include <stdarg.h>

#include <Arduino.h>


void com_init() {
	Serial.begin(115200);
}

void com_print(char *fmt, ...) {
	int meta;
	va_list ap;

	if(!fmt) return;

	va_start(ap,fmt);

	for(meta = 0; *fmt; fmt++)
		if(meta) {
			switch(*fmt) {
			case 'i': Serial.print(va_arg(ap,int)); break;
			case 'f': Serial.print(va_arg(ap,double),8); break;
			case '%':
			default: Serial.write(*fmt); break;
			}

			meta = 0;
		} else switch(*fmt) {
			case '%': meta = 1; break;
			case '\n': Serial.println(); break;
			default: Serial.write(*fmt); break;
			}

	va_end(ap);
}

