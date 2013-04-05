#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aux.h"
#include "car.h"
#include "event.h"
#include "font.h"
#include "gui.h"


void die(char *msg, ...)
{
	va_list ap;

	va_start(ap,msg);

	fprintf(stderr,"error: ");
	vfprintf(stderr,msg,ap);
	fprintf(stderr,"\n");

	va_end(ap);

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	font_init();

	aux_init();
	gui_init();

	car_init();

	event_loop();

	car_stop();

	gui_stop();
	aux_stop();

	font_stop();

	return 0;
}

