enum gui_event {
	GUI_PRESS,
	GUI_RELEASE,
	GUI_TIMEOUT,

	GUI_NUM_EVENTS
};

typedef struct gui_obj gui_obj_t;


extern void gui_init();
extern void gui_stop();

extern gui_obj_t *gui_find_obj(char *name, gui_obj_t *root);
extern void gui_set_value(gui_obj_t *obj, double value);
extern void gui_draw();
extern void gui_bind(char *, void (*)());
extern void gui_handle_pointer(enum gui_event, int, int);
extern void gui_handle_resize(int, int);

