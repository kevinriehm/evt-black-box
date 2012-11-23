#include "angel.h"

#include <EGL/egl.h>
#include <X11/Xlib.h>

#ifndef WINDOWED
#define WINDOWED // Revert to default
#endif


// Initializes X and EGL
void display_init() {
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
	EGLDisplay edisplay;

#ifdef WINDOWED
	int screen;
	EGLint vid;
	Display xdisplay;
	Window root, window;
	XVisualInfo *vinfo, vtemplate;
	XSetWindowAttributes attributes;
#endif

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
#ifdef WINDOWED
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
#endif
		EGL_RED_SIZE,        8,
		EGL_GREEN_SIZE,      8,
		EGL_BLUE_SIZE,       8,
		EGL_NONE
	};

	// Get a rendering target ready
#ifdef WINDOWED
	xdisplay = XOpenDisplay(NULL);
	if(!xdisplay) die("XOpenDisplay() failed");

	edisplay = eglGetDisplay(xdisplay);
#else
	edisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif

	if(!eglInitialize(edisplay))
		die("eglInitialize() failed");
	if(!eglChooseConfig(edisplay,attribs,&config,1,NULL))
		die("eglChooseConfig() failed");

#ifdef WINDOWED
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

	context = eglCreateContext(edisplay,config,EGL_NO_CONFIG,NULL);
	if(!context) die("eglCreateContext() failed");

	eglMakeCurrent(edisplay,surface,surface,context);

	// Some clean-up
	XFree(vinfo);
#endif
}

