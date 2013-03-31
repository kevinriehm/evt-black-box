#define X11

#include <stdio.h>

#include <fcntl.h>

#include <EGL/egl.h>

#ifdef X11
#	include <X11/Xlib.h>
#	include <X11/Xutil.h>
#endif

#include "angel.h"


#ifdef X11
Display *xdisplay;
#endif

#ifdef X11
static Window window;	
#endif

EGLContext context;
EGLSurface surface;
EGLDisplay edisplay;


// Returns a pretty string name for the error
const char *egl_error_string() {
	switch(eglGetError()) {
	case EGL_SUCCESS: return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_DISPLAY";
	case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
	default: return ""; // ?
	}
}

// Buffer swap
void display_update() {
	eglSwapBuffers(edisplay,surface);
}

// Initializes X and EGL
void display_init(int width, int height) {
	EGLConfig config;
	EGLint numconfigs;

#ifdef X11
	EGLint vid;
	Window root;
	int numvinfo;
	XVisualInfo *vinfo, vtemplate;
	XSetWindowAttributes attributes;
#endif

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
#ifdef X11
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
#endif
		EGL_RED_SIZE,        1,
		EGL_GREEN_SIZE,      1,
		EGL_BLUE_SIZE,       1,
		EGL_NONE
	};

	// Get a rendering target ready
#ifdef X11
	xdisplay = XOpenDisplay(NULL);
	if(!xdisplay) die("XOpenDisplay() failed");

	edisplay = eglGetDisplay((EGLNativeDisplayType) xdisplay);
#else
	edisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif

	if(edisplay == EGL_NO_DISPLAY)
		die("eglGetDisplay() failed");

	if(!eglInitialize(edisplay,NULL,NULL))
		die("eglInitialize() failed");

	if(!eglChooseConfig(edisplay,attribs,&config,1,&numconfigs)
		|| numconfigs == 0)
		die("eglChooseConfig() failed");

#ifdef X11
	// Match the EGL config with an X visual id
	eglGetConfigAttrib(edisplay,config,EGL_NATIVE_VISUAL_ID,&vid);
	vtemplate.visualid = vid;
	vinfo = XGetVisualInfo(xdisplay,VisualIDMask,&vtemplate,&numvinfo);
	if(!vinfo) die("XGetVisualInfo() failed");

	// Create a rendering window
	root = DefaultRootWindow(xdisplay);

	attributes.background_pixel = 0;
	attributes.border_pixel = 0;
	attributes.colormap = XCreateColormap(xdisplay,root,vinfo->visual,
		AllocNone);
	attributes.event_mask = ButtonPressMask | ButtonReleaseMask
		| ExposureMask | KeyPressMask | StructureNotifyMask;

	window = XCreateWindow(xdisplay,root,0,0,width,height,0,vinfo->depth,
		InputOutput,vinfo->visual,
		CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
		&attributes);
	if(!window) die("XCreateWindow() failed");

	XMapWindow(xdisplay,window); // Put it on screen
	XStoreName(xdisplay,window,"Angel Display Test"); // Give it a name

	// Mash X and EGL together

	surface = eglCreateWindowSurface(edisplay,config,
		(EGLNativeWindowType) window,NULL);
	if(surface == EGL_NO_SURFACE)
		die("eglCreateWindowSurface() failed");

	// Create an EGL context for OpenVG
	if(!eglBindAPI(EGL_OPENVG_API))
		die("eglBindAPI() failed");
	context = eglCreateContext(edisplay,config,EGL_NO_CONTEXT,NULL);
	if(context == EGL_NO_CONTEXT)
		die("eglCreateContext() failed");

	if(!eglMakeCurrent(edisplay,surface,surface,context))
		die("eglMakeCurrent() failed");

	// Some clean-up
	XFree(vinfo);
#endif
}

void display_stop() {
	eglMakeCurrent(edisplay,0,0,EGL_NO_CONTEXT);
	eglDestroyContext(edisplay,context);
	eglDestroySurface(edisplay,surface);
	eglTerminate(edisplay);

#ifdef X11
	XDestroyWindow(xdisplay,window);
	XCloseDisplay(xdisplay);
#endif
}

