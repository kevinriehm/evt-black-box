// Communication with an auxiliary board via serial

#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "angel.h"


#define PORT "/dev/ttyACM0"
#define BAUD B115200


int portfd;


void serial_init()
{
	clock_t c;
	char buf[5];
	int retries;
	struct termios portios;
	
	// Open the port
	portfd = open(PORT,O_RDWR | O_NOCTTY | O_NDELAY);
	if(portfd < 0) die("cannot open serial port '" PORT "'");
	
	// Set up the port
	memset(&portios,0,sizeof(portios));
	cfsetospeed(&portios,BAUD);
	cfsetispeed(&portios,BAUD);
	portios.c_cflag |= CS8;
	portios.c_iflag |= IGNBRK;
	portios.c_lflag |= NOFLSH;
	tcsetattr(portfd,TCSANOW,&portios);

	// Make sure it's working
	retries = 10;
	while(retries--) {
		write(portfd,"?\r\n",3);
		c = clock();
		while((clock() - c) < CLOCKS_PER_SEC
			|| strstr(buf,"OK"))
			read(portfd,buf,3);
	};
	if(strstr(buf,"OK"))
		die("cannot communciate with auxiliary board");
}

