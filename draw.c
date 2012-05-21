#include "angel.h"


static void begin_draw(draw_task_t *task);
static void finish_draw(draw_task_t *task);


draw_task_funcs_t begin_draw_funcs = {
	.draw = begin_draw
};

draw_task_funcs_t finish_draw_funcs = {
	.draw = finish_draw
};

SDL_Surface *screen;
float screenhscale, screenvscale;

static draw_task_t *tasks;


void draw_init()
{
	int i;
	draw_task_t *prevtask;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	
	if(!SDL_GetVideoInfo())
		die("apparently there is no display");
	
	SDL_WM_SetCaption("EVT Angel Display","EVT Angel Display");
	
	screen = SDL_SetVideoMode(800,380,0,0);
	if(!screen) die(SDL_GetError());
	
	screenhscale = (float) screen->w/640;
	screenvscale = (float) screen->h/480;
	
	for(prevtask = NULL, i = 0; draw_task_specs[i].funcs; i++)
	{
		draw_task_t *newtask = calloc(1,sizeof(draw_task_t));
		
		newtask->area.x = draw_task_specs[i].x*screen->w;
		newtask->area.y = draw_task_specs[i].y*screen->h;
		newtask->area.w = draw_task_specs[i].w*screen->w;
		newtask->area.h = draw_task_specs[i].h*screen->h;
		newtask->funcs = draw_task_specs[i].funcs;
		newtask->next = NULL;
		
		if(prevtask)
			prevtask = prevtask->next = newtask;
		else tasks = prevtask = newtask;
	}
}

static void begin_draw(draw_task_t *task)
{
	SDL_FillRect(screen,&task->area,SDL_MapRGB(screen->format,0xFF,0xFF,0xFF));
}

static void finish_draw(draw_task_t *task)
{
	SDL_Flip(screen);
}

void draw_screen()
{
	draw_task_t *task;
	
	for(task = tasks; task; task = task->next)
	{
		SDL_SetClipRect(screen,&task->area);
		if(task->funcs->draw)
			task->funcs->draw(task);
	}
	SDL_SetClipRect(screen,NULL);
}
