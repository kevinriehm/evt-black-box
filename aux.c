// Communication with an auxiliary board via serial

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "angel.h"


#define BUF_SIZE 100

#define PORT "/dev/ttyACM0"
#define BAUD B115200


int auxfd;


void aux_init()
{
	int n;
	clock_t c;
	int retries;
	char buf[BUF_SIZE];
	struct termios portios;
	
	// Open the port
	auxfd = open(PORT,O_RDWR | O_NOCTTY | O_NDELAY);
	if(auxfd < 0) die("cannot open serial port '" PORT "'");
	
	// Set up the port
	memset(&portios,0,sizeof(portios));
	cfsetospeed(&portios,BAUD);
	cfsetispeed(&portios,BAUD);
	portios.c_cflag |= CS8;
	portios.c_iflag |= IGNBRK;
	portios.c_lflag |= NOFLSH;
	tcsetattr(auxfd,TCSANOW,&portios);

	// Make sure it's working
	retries = 10;
	while(retries--) {
		printf("connecting to auxiliary, attempt %i...\n",10 - retries);
		write(auxfd,"?\r\n",3);
		c = clock();
		while((clock() - c) < CLOCKS_PER_SEC) {
			ioctl(auxfd,FIONREAD,&n);
			read(auxfd,buf,n < BUF_SIZE ? n : BUF_SIZE - 1);
			buf[BUF_SIZE - 1] = '\0';

			if(strstr(buf,"OK")) goto connected;
		}
	};
	die("cannot communciate with auxiliary board");

connected:
	puts("connected to auxiliary board");
}

void aux_stop() {
	close(auxfd);
}

