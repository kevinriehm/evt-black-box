// Communication with an auxiliary board via serial

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "main.h"


#define PORT "/dev/ttyACM0"
#define BAUD B9600


static int portfd;


void serial_cmd(char *result, int max, char *cmd)
{
	int i;
	write(portfd,cmd,strlen(cmd));
	do {
		for(i = 1000; i && read(portfd,result,1) != 1; usleep(1000), i--);
	} while(--max && *(result++) && i);
	if(max) *result = '\0';
}

void serial_init()
{
	int retries;
	char str[10];
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
	
	// Check everything is working
	retries = 10;
	do serial_cmd(str,sizeof(str),"?");
	while(--retries && strcmp(str,"OK") != 0);
	if(!retries) die("cannot reach auxiliary board serial port");
}

