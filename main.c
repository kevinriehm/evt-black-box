#include "angel.h"


void die(char *msg)
{
	printf("error: %s\n",msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	serial_init();
	data_init();
	log_init();
	
	display_init();
	gui_init();
	
	event_loop();
	
	log_stop();
	data_stop();

	return EXIT_SUCCESS;
}
