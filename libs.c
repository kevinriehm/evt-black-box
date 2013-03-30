#define _LIBS_C_

#include <dlfcn.h>

#include <X11/Xlib.h>

#include "angel.h"
#include "libs.h"


#define LIB(name) curlib = lib##name = load_lib("lib"#name".so")

#define SYM(name) l_##name = load_sym(curlib,#name)


static void *libEGL;
static void *libfreetype;
static void *libm;
static void *libOpenVG;
static void *librt;
static void *libX11;


static void *load_lib(char *name) {
	void *lib = dlopen(name,RTLD_LAZY);
	if(!lib) die("cannot load library '%s' (%s)",name,dlerror());
	return lib;
}

static void *load_sym(void *library, char *name) {
	void *sym = dlsym(library,name);
	if(!sym) die("cannot resolve symbol '%s' (%s)",name,dlerror());
	return sym;
}

void libs_init() {
	void *curlib;

	LIB(EGL);
	SYM(eglBindAPI);
	SYM(eglChooseConfig);
	SYM(eglCreateContext);
	SYM(eglCreateWindowSurface);
	SYM(eglDestroyContext);
	SYM(eglDestroySurface);
	SYM(eglGetConfigAttrib);
	SYM(eglGetDisplay);
	SYM(eglGetError);
	SYM(eglInitialize);
	SYM(eglMakeCurrent);
	SYM(eglSwapBuffers);
	SYM(eglTerminate);

	LIB(freetype);
	SYM(FT_Done_Face);
	SYM(FT_Done_FreeType);
	SYM(FT_Init_FreeType);
	SYM(FT_Load_Char);
	SYM(FT_New_Face);
	SYM(FT_Outline_Decompose);
	SYM(FT_Select_Charmap);
	SYM(FT_Set_Pixel_Sizes);

	LIB(m);
	SYM(cos);
	SYM(sin);

	LIB(OpenVG);
	SYM(vgAppendPathData);
	SYM(vgClear);
	SYM(vgCreateFont);
	SYM(vgCreatePaint);
	SYM(vgCreatePath);
	SYM(vgDestroyPaint);
	SYM(vgDestroyPath);
	SYM(vgDrawGlyph);
	SYM(vgDrawPath);
	SYM(vgGetColor);
	SYM(vgGetError);
	SYM(vgGetMatrix);
	SYM(vgGetParameteri);
	SYM(vgLoadIdentity);
	SYM(vgLoadMatrix);
	SYM(vgMultMatrix);
	SYM(vgPathBounds);
	SYM(vgPathTransformedBounds);
	SYM(vgRotate);
	SYM(vgScale);
	SYM(vgSetColor);
	SYM(vgSetf);
	SYM(vgSetfv);
	SYM(vgSetGlyphToPath);
	SYM(vgSeti);
	SYM(vgSetPaint);
	SYM(vgSetParameterfv);

	LIB(rt);
	SYM(clock_gettime);

	LIB(X11);
	SYM(XCloseDisplay);
	SYM(XCreateWindow);
	SYM(XDestroyWindow);
	SYM(XFree);
	SYM(XGetVisualInfo);
	SYM(XLookupString);
	SYM(XMapWindow);
	SYM(XNextEvent);
	SYM(XOpenDisplay);
	SYM(XStoreName);
}

void libs_stop() {
	dlclose(libX11);
	dlclose(librt);
	dlclose(libOpenVG);
	dlclose(libm);
	dlclose(libEGL);
}

