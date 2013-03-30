#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "display.h"
#include "event.h"
#include "gui.h"
#include "libs.h"


int wantdraw = 1;


void event_loop()
{
	char c;
	XEvent event;
	int status, quit;
	struct pollfd fds[1];
	XButtonEvent *button;
	XConfigureEvent *configure;

	// Set up the X11 events connection for poll
	fds[0].fd = ConnectionNumber(xdisplay);
	fds[0].events = POLLIN;

	quit = 0;

	// Shortcuts
	button = (XButtonEvent *) &event;
	configure = (XConfigureEvent *) &event;

	// Handle events! Yay!
	do {
		status = poll(fds,1,wantdraw ? 0 : -1); // Wait for events...

		if(status < 0) // Error; ignore it
			continue;

		if(status == 0) { // Nothing happened, just refreshin'
			gui_draw();
			wantdraw = 0;
		}

		// Something happened!

		XNextEvent(xdisplay,&event);

		switch(event.type) {
		case ButtonPress:
			gui_handle_pointer(GUI_PRESS,button->x,button->y);
			break;

		case ButtonRelease:
			gui_handle_pointer(GUI_RELEASE,button->x,button->y);
			break;

		case ConfigureNotify:
			gui_handle_resize(configure->width,configure->height);
			break;

		case Expose:
			event_redraw();
			break;

		case KeyPress:
			XLookupString(&event.xkey,&c,1,NULL,NULL);

			switch(c) {
			case 0x1B: quit = 1; break; // Escape
			}
			break;
		}
	} while(!quit);
}

void event_redraw() {
	wantdraw = 1;
}

