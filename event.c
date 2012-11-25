#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "display.h"


#define MAX_FPS 30


void event_loop()
{
	char c;
	int quit;
	XEvent event;
	
	// Handle events! Yay!
	do {
		XNextEvent(xdisplay,&event);

		switch(event.type) {
		case KeyPress:
			XLookupString(&event.xkey,&c,1,NULL,NULL);
			fprintf(stderr,"%02x\n",c);

			switch(c) {
			case 0x1B: quit = 1; break; // Escape
			}
			break;
		}
	} while(!quit);
}
