#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "car.h"
#include "display.h"
#include "event.h"
#include "gui.h"


#define BUF_SIZE 0x1000


int wantdraw = 1;


void event_loop()
{
	static char buf[BUF_SIZE];

	char c;
	int nread;
	XEvent event;
	int status, quit;
	struct pollfd fds[2];
	XButtonEvent *button;
	XConfigureEvent *configure;

	// Set up the X11 events connection for poll
	fds[0].fd = ConnectionNumber(xdisplay);
	fds[0].events = POLLIN;

	// Set up monitoring for the Arduino
	fds[1].fd = open("/dev/ttyACM0",O_RDWR);
	fds[1].events = POLLIN;

	quit = 0;

	// Shortcuts
	button = (XButtonEvent *) &event;
	configure = (XConfigureEvent *) &event;

	// Handle events! Yay!
	do {
		status = poll(fds,2,wantdraw ? 0 : -1); // Wait for events...

		if(status < 0) // Error; ignore it
			continue;

		if(status == 0) { // Nothing happened, just refreshin'
			gui_draw();
			wantdraw = 0;
		}

		// Something happened!

		if(!(fds[0].revents & POLLIN)) goto no_x_event;

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
no_x_event:

		if(!(fds[1].revents & POLLIN)) goto no_aux_data;

		// Get a full data block
		for(nread = 0; nread == 0 || buf[nread] != '}';) {
			nread += read(fds[1].fd,buf + nread,1);

			if(nread >= BUF_SIZE) { // Oh no!
				printf("auxiliary data buffer maxed-out\n");
				break;
			}
		}

		// Process the data block
		car_handle_data_block(buf);

no_aux_data:
		;
	} while(!quit);
}

void event_redraw() {
	wantdraw = 1;
}

