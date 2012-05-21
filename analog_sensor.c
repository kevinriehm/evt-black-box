#include "angel.h"


static void analog_draw(draw_task_t *task);

draw_task_funcs_t analog_draw_funcs = {
	.draw = analog_draw
};

static void analog_draw(draw_task_t *task)
{
	char valstr[20];
	serial_cmd(valstr,20,"a0");
	print_text(valstr,task->area.x,task->area.y,20,20,0,0,0);
}
