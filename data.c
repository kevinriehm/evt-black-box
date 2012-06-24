#include "angel.h"

#include <SDL_thread.h>


#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define LOCAL_SERVER "laptop:8080"

#define SAMPLE_DELAY_MS 100


struct datum {
	int64_t time;
	uint16_t potentiometer;
};


static int datastop;
static SDL_Thread *datathread;

static SDL_mutex *datummutex;
static struct datum currentdatum;


static int data_thread(void *data)
{
	struct datum datum;
	uint32_t ticks = SDL_GetTicks(),
		delay;
	
	do {
		SDL_LockMutex(datummutex);
		currentdatum = datum;
		SDL_UnlockMutex(datummutex);
		
		if((delay = SAMPLE_DELAY_MS - (SDL_GetTicks() - ticks)) > 0)
			SDL_Delay(delay);
		ticks = MAX(ticks + SAMPLE_DELAY_MS,SDL_GetTicks());
	} while(!datastop);
	
	return 0;
}

void data_init()
{
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
