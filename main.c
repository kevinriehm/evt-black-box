#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aux.h"
#include "event.h"
#include "gui.h"
#include "libs.h"


void die(char *msg, ...)
{
	va_list ap;

	va_start(ap,msg);

	fprintf(stderr,"error:");
	vfprintf(stderr,msg,ap);
	fprintf(stderr,"\n");

	va_end(ap);

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	libs_init();

	aux_init();
	gui_init();

	event_loop();

	gui_stop();
	aux_stop();

	libs_stop();

	return 0;
}

