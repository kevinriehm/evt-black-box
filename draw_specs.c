#include "angel.h"


const draw_task_spec_t draw_task_specs[] = {
	{&begin_draw_funcs,0,0,1,1,NULL},
	{&clock_draw_funcs,0,0,0.5,0.2,NULL},
	{&analog_draw_funcs,0.0,0.45,1.0,0.1,"style = bar; smooth = ema;"},
	{&finish_draw_funcs,0,0,1,1,NULL},
	{NULL}
};
