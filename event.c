#include "angel.h"


#define DRAW_DELAY_MS 30


enum {
	EVENT_UPDATETRIGGER = SDL_USEREVENT
};


// Triggers the regular screen update
Uint32 update_timer_callback(Uint32 interval, void *param)
{
	SDL_Event event;
	if(SDL_PeepEvents(&event,1,SDL_PEEKEVENT,SDL_EVENTMASK(EVENT_UPDATETRIGGER)) < 1
		|| event.type != EVENT_UPDATETRIGGER)
	{
		event.type = EVENT_UPDATETRIGGER;
		SDL_PushEvent(&event);
	}
	return interval;
}

void event_loop()
{
	int quit = 0;
	SDL_Event event;
	
	// Set up the regular screen update
	SDL_AddTimer(DRAW_DELAY_MS,update_timer_callback,NULL);
	
	// Handle events! Yay!
	while(SDL_WaitEvent(&event) && !quit)
	{
		switch(event.type)
		{
			case SDL_KEYUP:
				switch(event.key.keysym.sym)
				{
					case SDLK_ESCAPE: quit = 1; break;
					default: break;
				}
				break;
			
			case SDL_QUIT: quit = 1; break;
			
			case EVENT_UPDATETRIGGER: draw_screen(); break;
		}
	}
}
