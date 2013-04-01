#include <stdarg.h>

#include <Arduino.h>


#define OUTPUT(args...) { \
	Serial.print(args); \
	Serial2.print(args); \
}


void com_init() {
	Serial.begin(115200);
	Serial.setTimeout(100);

	Serial2.begin(9600);
}

char com_read_cmd() {
//	if(!Serial.available())
//		return '\0';
	return Serial.read();
}

int com_read_int() {
//	if(!Serial.available())
//		return 0;
	return Serial.parseInt();
}

void com_print(char *fmt, ...) {
	double d;
	va_list ap;
	int meta, i;

	if(!fmt) return;

	va_start(ap,fmt);

	for(meta = 0; *fmt; fmt++)
		if(meta) {
			switch(*fmt) {
			case 'i': i = va_arg(ap,int); OUTPUT(i); break;
			case 'f': d = va_arg(ap,double); OUTPUT(d,8); break;
			case '%':
			default: OUTPUT(*fmt); break;
			}

			meta = 0;
		} else switch(*fmt) {
			case '%': meta = 1; break;
			case '\n': OUTPUT("\r\n"); break;
			default: OUTPUT(*fmt); break;
			}

	va_end(ap);
}

