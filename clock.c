#include "angel.h"


static void clock_draw(draw_task_t *task);

draw_task_funcs_t clock_draw_funcs = {
	.draw = clock_draw
};

static void clock_draw(draw_task_t *task)
{
	char timestr[20];
	time_t curtime = time(NULL);
	strftime(timestr,20,"%I:%M.%S %p",localtime(&curtime));
	print_text(timestr,task->area.x,task->area.y,100,200,0,0,0);
}
