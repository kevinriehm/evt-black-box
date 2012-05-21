/*
 * args:
 *  - style = [text|bar];
 */

#include "angel.h"


#define MAX_VAL 1023


static void analog_draw_text(draw_task_t *task)
{
	char valstr[20];	
	serial_cmd(valstr,20,"a0");
	print_text(valstr,task->area.x,task->area.y,50,50,0,0,0);
}

static void analog_draw_bar(draw_task_t *task)
{
	int val;
	char valstr[20];
	
	serial_cmd(valstr,20,"a0");
	val = atoi(valstr);
	
	// Border, fill
	rectangleRGBA(screen,task->area.x,task->area.y,
		task->area.x + task->area.w - 1,task->area.y + task->area.h - 1,
		50,50,255,255);
	boxRGBA(screen,task->area.x + 1,task->area.y + 1,
		task->area.x + (task->area.w - 2)*val/MAX_VAL,task->area.y + task->area.h - 2,
		255,0,0,255);
}

static void analog_init(draw_task_t *task, char *args)
{
	char attr[20], val[20];
	
	// Defaults
	task->funcs.draw = analog_draw_text;
	
	// Process args
	while(sscanf(args,"%19s = %19[^;];",attr,val) == 2 && (args = strchr(args,';')))
	{
		if(strcmp(attr,"style") == 0)
		{
			if(strcmp(val,"text") == 0)
				task->funcs.draw = analog_draw_text;
			else if(strcmp(val,"bar") == 0)
				task->funcs.draw = analog_draw_bar;
		}
	}
}

draw_task_funcs_t analog_draw_funcs = {
	.init = analog_init
};
