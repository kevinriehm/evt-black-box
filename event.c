#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "display.h"
#include "gui.h"


#define MAX_FPS 30


static uint64_t timespec_to_ms(struct timespec *time) {
	return (uint64_t) time->tv_sec*1000 + (uint64_t) time->tv_nsec/1000000;
}

void event_loop()
{
	char c;
	XEvent event;
	struct pollfd fds[1];
	int nextdrawms, status, quit;
	struct timespec lastdraw, now;

	// Set up the X11 events connection for poll
	fds[0].fd = ConnectionNumber(xdisplay);
	fds[0].events = POLLIN;

	clock_gettime(CLOCK_MONOTONIC,&lastdraw);

	quit = 0;

	// Handle events! Yay!
	do {
		// Find out how long until the next screen draw
		clock_gettime(CLOCK_MONOTONIC,&now);
		nextdrawms = 1000/MAX_FPS
			- (timespec_to_ms(&now) - timespec_to_ms(&lastdraw));
		if(nextdrawms < 0) nextdrawms = 0;

		status = poll(fds,1,nextdrawms); // Wait for events...

		if(status < 0) // Error; ignore it
			continue;

		if(status == 0) { // Nothing happened; just refreshin' the screen
			clock_gettime(CLOCK_MONOTONIC,&lastdraw);
			gui_draw();
			continue;
		}

		// Something happened!

		XNextEvent(xdisplay,&event);

		switch(event.type) {
		case KeyPress:
			XLookupString(&event.xkey,&c,1,NULL,NULL);

			switch(c) {
			case 0x1B: quit = 1; break; // Escape
			}
			break;
		}
	} while(!quit);
}

