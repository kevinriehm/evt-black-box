enum gui_event {
	GUI_PRESS,
	GUI_RELEASE,

	GUI_NUM_EVENTS
};


extern void gui_init();
extern void gui_stop();

extern void gui_draw();
extern void gui_handle_pointer(enum gui_event, int, int);
extern void gui_handle_resize(int, int);

