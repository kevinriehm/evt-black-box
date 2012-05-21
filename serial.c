// Communication with an auxiliary board via serial

#include "angel.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>


#define PORT "/dev/ttyACM0"
#define BAUD B115200


static int portfd;


void serial_init()
{
	struct termios portios;
	
	// Open the port
	portfd = open(PORT,O_RDWR | O_NOCTTY);
	if(!portfd) die("cannot open serial port '" PORT "'");
	
	// Set up the port
	memset(&portios,0,sizeof(portios));
	cfsetospeed(&portios,BAUD);
	cfsetispeed(&portios,BAUD);
	portios.c_cflag |= CS8;
	portios.c_iflag |= IGNBRK;
	portios.c_lflag |= NOFLSH;
	tcsetattr(portfd,TCSANOW,&portios);
}

void serial_cmd(char *result, int n, char *cmd)
{
	write(portfd,cmd,strlen(cmd));
	do {
		while(read(portfd,result,1) != 1);
	} while(--n && (result++)[0]);
}
