#include <EGL/egl.h>
#include <X11/Xlib.h>


extern Display *xdisplay;

extern EGLContext context;
extern EGLSurface surface;
extern EGLDisplay edisplay;

extern void display_init(int, int);
extern void display_stop();

extern void display_update();

