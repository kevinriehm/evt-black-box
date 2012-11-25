#ifndef X11
#define X11 // Revert to default
#endif

#include <EGL/egl.h>

#ifdef X11
#	include <X11/Xlib.h>
#endif

#include "main.h"


int screenwidth = 640;
int screenheight = 480;

static EGLContext context;
static EGLSurface surface;
static EGLDisplay edisplay;

#ifdef X11
static Window window;	
static Display *xdisplay;
#endif


// Initializes X and EGL
void display_init() {
	EGLConfig config;

#ifdef X11
	int screen;
	EGLint vid;
	Window root;
	XVisualInfo *vinfo, vtemplate;
	XSetWindowAttributes attributes;
#endif

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
#ifdef X11
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
#endif
		EGL_RED_SIZE,        8,
		EGL_GREEN_SIZE,      8,
		EGL_BLUE_SIZE,       8,
		EGL_NONE
	};

	// Get a rendering target ready
#ifdef X11
	xdisplay = XOpenDisplay(NULL);
	if(!xdisplay) die("XOpenDisplay() failed");

	edisplay = eglGetDisplay(xdisplay);
#else
	edisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif

	if(!eglInitialize(edisplay,NULL,NULL))
		die("eglInitialize() failed");
	if(!eglChooseConfig(edisplay,attribs,&config,1,NULL))
		die("eglChooseConfig() failed");

#ifdef X11
	// Match the EGL config with an X visual id
	eglGetConfigAttrib(edisplay,config,EGL_NATIVE_VISUAL_ID,&vid);
	vtemplate.visualid = vid;
	vinfo = XGetVisualInfo(xdisplay,VisualIDMask,&vtemplate,NULL);
	if(!vinfo) die("XGetVisualInfo() failed");

	// Create a rendering window
	screen = DefaultScreen(xdisplay);
	root = RootWindow(xdisplay,screen);

	attributes.event_mask = KeyPressMask | StructureNotifyMask;

	window = XCreateWindow(xdisplay,root,0,0,screenwidth,screenheight,0,
		vinfo->depth,InputOutput,vinfo->visual,CWEventMask,&attributes);
	XMapWindow(xdisplay,window);

	// Mash X and EGL together
	surface = eglCreateWindowSurface(edisplay,config,window,NULL);
	if(!surface) die("eglCreateWindowSurface() failed");

	context = eglCreateContext(edisplay,config,EGL_NO_CONTEXT,NULL);
	if(!context) die("eglCreateContext() failed");

	eglMakeCurrent(edisplay,surface,surface,context);

	// Some clean-up
	XFree(vinfo);
#endif
}

void display_stop() {
	eglMakeCurrent(edisplay,0,0,NULL);
	eglDestroyContext(edisplay,context);
	eglDestroySurface(edisplay,surface);

#ifdef X11
	XDestroyWindow(xdisplay,window);
	XCloseDisplay(xdisplay);
#endif
}

