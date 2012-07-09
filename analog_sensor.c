/*
 * args:
 *  - style = [text|bar];
 *  - smooth = [none|ema];
 *  - value = [time|potentiometer]
 */

#include "angel.h"


#define MAX_VAL 1023


typedef enum {
	SMOOTH_NONE,
	SMOOTH_EMA // Exponential moving average
} smooth_type_t;

typedef struct {
	int oldval;
	smooth_type_t smooth;
	datum_value_id_t valueid;
} analog_data_t;


static int get_value(draw_task_t *task)
{
	uint16_t val;
	analog_data_t *data = task->data;
	
	data_get_value(&val,data->valueid);
	
	switch(data->smooth)
	{
		case SMOOTH_NONE: break;
		case SMOOTH_EMA: val = data->oldval = (data->oldval + val + 1)/2; break;
	}
	
	return val;
}

static void analog_draw_text(draw_task_t *task)
{
	char str[20];
	sprintf(str,"%i",get_value(task));
	print_text(str,task->area.x,task->area.y,50,50,0,0,0);
}

static void analog_draw_bar(draw_task_t *task)
{
	int val = get_value(task);
	
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
	
	task->data = calloc(1,sizeof(analog_data_t));
	
	// Defaults
	task->funcs.draw = analog_draw_text;
	((analog_data_t *) task->data)->smooth = SMOOTH_NONE;
	
	// Process args
	while(sscanf(args,"%19s = %19[^;];",attr,val) == 2 && (args = strchr(args,';')) && args++)
	{
		if(strcmp(attr,"style") == 0)
		{
			if(strcmp(val,"text") == 0)
				task->funcs.draw = analog_draw_text;
			else if(strcmp(val,"bar") == 0)
				task->funcs.draw = analog_draw_bar;
		}
		else if(strcmp(attr,"smooth") == 0)
		{
			if(strcmp(val,"none") == 0)
				((analog_data_t *) task->data)->smooth = SMOOTH_NONE;
			else if(strcmp(val,"ema") == 0)
				((analog_data_t *) task->data)->smooth = SMOOTH_EMA;
		}
		else if(strcmp(attr,"value") == 0)
		{
			((analog_data_t *) task->data)->valueid = data_get_id(val);
		}
	}
}

draw_task_funcs_t analog_draw_funcs = {
	.init = analog_init
};
