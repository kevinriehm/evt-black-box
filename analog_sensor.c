#include "angel.h"


#define MAX_VAL 1023


static void analog_draw(draw_task_t *task);

draw_task_funcs_t analog_draw_funcs = {
	.draw = analog_draw
};

static void analog_draw(draw_task_t *task)
{
	int val;
	char valstr[20];
	
	serial_cmd(valstr,20,"a0");
	print_text(valstr,task->area.x,task->area.y,50,50,0,0,0);
	
	val = atoi(valstr);
	rectangleRGBA(screen,task->area.x,task->area.y + task->area.h/2,
		task->area.x + task->area.w - 1,task->area.y + task->area.h/2 + 50,
		50,50,255,255);
	boxRGBA(screen,task->area.x + 1,task->area.y + task->area.h/2 + 1,
		task->area.x + (task->area.w - 2)*val/MAX_VAL,
		task->area.y + task->area.h/2 + 49,255,0,0,255);
}
