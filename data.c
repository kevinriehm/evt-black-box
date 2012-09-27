#include "angel.h"


#define SAMPLE_DELAY_MS 100


static datum_t currentdatum;
static SDL_mutex *datummutex;

static int datastop;
static time_t unixepoch;
static SDL_Thread *datathread;


static int data_thread(void *data)
{
	char buf[31];
	datum_t datum;
	uint32_t ticks = SDL_GetTicks();
	
	do {
		// Get the data
		datum.time = difftime(time(NULL),unixepoch);
		
		serial_cmd(buf,30,"a0");
		datum.potentiometer = atoi(buf);
		
		serial_cmd(buf,30,"gp");
		sscanf(buf,"%f,%f",&datum.latitude,&datum.longitude);
		
		// Store it in a thread-safe manner
		SDL_LockMutex(datummutex);
		currentdatum = datum;
		SDL_UnlockMutex(datummutex);
		
		// Make sure the timing is as perfect as possible
		if(ticks + SAMPLE_DELAY_MS > SDL_GetTicks())
		{
			SDL_Delay(SAMPLE_DELAY_MS - (SDL_GetTicks() - ticks));
			ticks += SAMPLE_DELAY_MS;
			while(SDL_GetTicks() < ticks); // Just to make sure
		} else // Took too long...*sob*
			ticks = SDL_GetTicks();
	} while(!datastop);
	
	return 0;
}

void data_init()
{
	struct tm epoch = {
		.tm_year = 70, // 1970
		.tm_mon = 0,   // January
		.tm_mday = 1,  // 1st
		.tm_hour = 0,  // 00
		.tm_min = 0,   //   :00
		.tm_sec = 0    //      :00
	};
	
	unixepoch = mktime(&epoch);
	
	// Spin off another thread to keep stuff as in sync as possible
	datastop = 0;
	datummutex = SDL_CreateMutex();
	datathread = SDL_CreateThread(data_thread,NULL);
}

void data_stop()
{
	datastop = 1;
	SDL_Delay(SAMPLE_DELAY_MS + 1);
	SDL_KillThread(datathread); // If it froze, don't let that spread
}

void data_get(datum_t *datum)
{
	SDL_LockMutex(datummutex);
	*datum = currentdatum;
	SDL_UnlockMutex(datummutex);
}

datum_value_id_t data_get_id(char *value)
{
	if(strcmp(value,"time") == 0) return DATUM_TIME;
	else if(strcmp(value,"potentiometer") == 0) return DATUM_POTENTIOMETER;
	else return DATUM_INVALID;
}

void data_get_value(void *value, datum_value_id_t id)
{
	SDL_LockMutex(datummutex);
	switch(id)
	{
		case DATUM_TIME: *(uint64_t *) value = currentdatum.time; break;
		case DATUM_POTENTIOMETER: *(uint16_t *) value = currentdatum.potentiometer; break;
		case DATUM_INVALID: break;
	}
	SDL_UnlockMutex(datummutex);
}
