#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "angel.h"
#include "aux.h"
#include "car.h"
#include "display.h"
#include "event.h"
#include "gui.h"


#define BUF_SIZE 0x1000

#define PORT "/dev/ttyACM0"
#define BAUD B115200


static int wantdraw = 1;
static int maxwait = -1;


void event_loop()
{
	static char buf[BUF_SIZE];

	char c;
	int n, nread;
	XEvent event;
	int status, quit;
	struct pollfd fds[2];
	XButtonEvent *button;
	XConfigureEvent *configure;

	// Set up the X11 events connection for poll
	fds[0].fd = ConnectionNumber(xdisplay);
	fds[0].events = POLLIN;

	// Set up monitoring for the Arduino
	fds[1].fd = auxfd;
	fds[1].events = POLLIN;
	nread = 0;

	quit = 0;

	// Shortcuts
	button = (XButtonEvent *) &event;
	configure = (XConfigureEvent *) &event;

	// Handle events! Yay!
	do {
		status = poll(fds,2,wantdraw ? 0 : maxwait); // Wait for events

		if(status < 0) // Error; ignore it
			continue;

		if(status == 0) { // Nothing happened, just refreshin'
			maxwait = -1;
			gui_draw();
			wantdraw = 0;
			continue;
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

		ioctl(auxfd,FIONREAD,&n);
		if(n <= 0) goto no_aux_data;

		// Get a full data block
		nread += read(auxfd,buf + nread,n);
		buf[nread] = '\0';

		// Process the data block
		if(strchr(buf,'{') && strchr(buf,'}')) {
			car_handle_data_block(strchr(buf,'{'));
			nread = 0;
		}

no_aux_data:
		;
	} while(!quit);
}

void event_redraw() {
	wantdraw = 1;
}

void event_set_max_wait(float seconds) {
	maxwait = seconds >= 0 && (maxwait < 0 || seconds < maxwait)
		? seconds*1000 : maxwait;
}

