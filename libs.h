#include <EGL/egl.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <VG/openvg.h>
#include <X11/Xlib.h>

#ifdef _LIBS_C_
#	define EXTERN
#else
#	define EXTERN extern
#endif

#define LIB_FUNC(type, name, args...) EXTERN type (*l_##name)(args)


#define eglBindAPI              l_eglBindAPI
#define eglChooseConfig         l_eglChooseConfig
#define eglCreateContext        l_eglCreateContext
#define eglCreateWindowSurface  l_eglCreateWindowSurface
#define eglDestroyContext       l_eglDestroyContext
#define eglDestroySurface       l_eglDestroySurface
#define eglGetConfigAttrib      l_eglGetConfigAttrib
#define eglGetDisplay           l_eglGetDisplay
#define eglGetError             l_eglGetError
#define eglInitialize           l_eglInitialize
#define eglMakeCurrent          l_eglMakeCurrent
#define eglSwapBuffers          l_eglSwapBuffers
#define eglTerminate            l_eglTerminate

#define FT_Done_Face            l_FT_Done_Face
#define FT_Done_FreeType        l_FT_Done_FreeType
#define FT_Init_FreeType        l_FT_Init_FreeType
#define FT_Load_Char            l_FT_Load_Char
#define FT_New_Face             l_FT_New_Face
#define FT_Outline_Decompose    l_FT_Outline_Decompose
#define FT_Select_Charmap       l_FT_Select_Charmap
#define FT_Set_Pixel_Sizes      l_FT_Set_Pixel_Sizes

#define cos                     l_cos
#define sin                     l_sin

#define vgAppendPathData        l_vgAppendPathData
#define vgClear                 l_vgClear
#define vgCreateFont            l_vgCreateFont
#define vgCreatePaint           l_vgCreatePaint
#define vgCreatePath            l_vgCreatePath
#define vgDestroyPaint          l_vgDestroyPaint
#define vgDestroyPath           l_vgDestroyPath
#define vgDrawGlyph             l_vgDrawGlyph
#define vgDrawPath              l_vgDrawPath
#define vgGetColor              l_vgGetColor
#define vgGetError              l_vgGetError
#define vgGetMatrix             l_vgGetMatrix
#define vgGetParameteri         l_vgGetParameteri
#define vgLoadIdentity          l_vgLoadIdentity
#define vgLoadMatrix            l_vgLoadMatrix
#define vgMultMatrix            l_vgMultMatrix
#define vgPathBounds            l_vgPathBounds
#define vgPathTransformedBounds l_vgPathTransformedBounds
#define vgRotate                l_vgRotate
#define vgScale                 l_vgScale
#define vgSetColor              l_vgSetColor
#define vgSetf                  l_vgSetf
#define vgSetfv                 l_vgSetfv
#define vgSetGlyphToPath        l_vgSetGlyphToPath
#define vgSeti                  l_vgSeti
#define vgSetPaint              l_vgSetPaint
#define vgSetParameterfv        l_vgSetParameterfv

#define clock_gettime           l_clock_gettime

#define XCloseDisplay           l_XCloseDisplay
#define XCreateWindow           l_XCreateWindow
#define XDestroyWindow          l_XDestroyWindow
#define XFree                   l_XFree
#define XGetVisualInfo          l_XGetVisualInfo
#define XLookupString           l_XLookupString
#define XMapWindow              l_XMapWindow
#define XNextEvent              l_XNextEvent
#define XOpenDisplay            l_XOpenDisplay
#define XStoreName              l_XStoreName


LIB_FUNC(EGLBoolean, eglBindAPI, EGLenum);
LIB_FUNC(EGLBoolean, eglChooseConfig, EGLDisplay, const EGLint *, EGLConfig *,
	EGLint, EGLint *);
LIB_FUNC(EGLContext, eglCreateContext, EGLDisplay, EGLConfig, EGLContext,
	const EGLint *);
LIB_FUNC(EGLSurface, eglCreateWindowSurface, EGLDisplay, EGLConfig,
	EGLNativeWindowType, const EGLint *);
LIB_FUNC(EGLBoolean, eglDestroyContext, EGLDisplay, EGLContext);
LIB_FUNC(EGLBoolean, eglDestroySurface, EGLDisplay, EGLSurface);
LIB_FUNC(EGLBoolean, eglGetConfigAttrib, EGLDisplay, EGLConfig, EGLint,
	EGLint *);
LIB_FUNC(EGLDisplay, eglGetDisplay, EGLNativeDisplayType);
LIB_FUNC(EGLint, eglGetError);
LIB_FUNC(EGLBoolean, eglInitialize, EGLDisplay, EGLint *, EGLint *);
LIB_FUNC(EGLBoolean, eglMakeCurrent, EGLDisplay, EGLSurface, EGLSurface,
	EGLContext);
LIB_FUNC(EGLBoolean, eglSwapBuffers, EGLDisplay, EGLSurface);
LIB_FUNC(EGLBoolean, eglTerminate, EGLDisplay);

LIB_FUNC(FT_Error, FT_Done_Face, FT_Face);
LIB_FUNC(FT_Error, FT_Done_FreeType, FT_Library);
LIB_FUNC(FT_Error, FT_Init_FreeType, FT_Library *);
LIB_FUNC(FT_Error, FT_Load_Char, FT_Face, FT_ULong, FT_Int32);
LIB_FUNC(FT_Error, FT_New_Face, FT_Library, const char *, FT_Long, FT_Face *);
LIB_FUNC(FT_Error, FT_Outline_Decompose, FT_Outline *,
	const FT_Outline_Funcs *, void *);
LIB_FUNC(FT_Error, FT_Select_Charmap, FT_Face, FT_Encoding);
LIB_FUNC(FT_Error, FT_Set_Pixel_Sizes, FT_Face, FT_UInt, FT_UInt);

LIB_FUNC(double, cos, double);
LIB_FUNC(double, sin, double);

LIB_FUNC(void, vgAppendPathData, VGPath, VGint, const VGubyte *, const void *);
LIB_FUNC(void, vgClear, VGint, VGint, VGint, VGint);
LIB_FUNC(VGFont, vgCreateFont, VGint);
LIB_FUNC(VGPaint, vgCreatePaint);
LIB_FUNC(VGPath, vgCreatePath, VGint, VGPathDatatype, VGfloat, VGfloat, VGint,
	VGint, VGbitfield);
LIB_FUNC(void, vgDestroyPaint, VGPaint);
LIB_FUNC(void, vgDestroyPath, VGPath);
LIB_FUNC(void, vgDrawGlyph, VGFont, VGuint, VGbitfield, VGboolean);
LIB_FUNC(VGfloat, vgDrawPath, VGPath, VGbitfield);
LIB_FUNC(VGuint, vgGetColor, VGPaint);
LIB_FUNC(VGErrorCode, vgGetError);
LIB_FUNC(void, vgGetMatrix, VGfloat *);
LIB_FUNC(VGint, vgGetParameteri, VGHandle, VGint);
LIB_FUNC(void, vgLoadIdentity);
LIB_FUNC(void, vgLoadMatrix, const VGfloat *);
LIB_FUNC(void, vgMultMatrix, const VGfloat *);
LIB_FUNC(void, vgPathBounds, VGPath, VGfloat *, VGfloat *, VGfloat *,
	VGfloat *);
LIB_FUNC(void, vgPathTransformedBounds, VGPath, VGfloat *, VGfloat *,
	VGfloat *, VGfloat *);
LIB_FUNC(void, vgRotate, VGfloat);
LIB_FUNC(void, vgScale, VGfloat, VGfloat);
LIB_FUNC(void, vgSetColor, VGPaint, VGuint);
LIB_FUNC(void, vgSetf, VGParamType, VGfloat);
LIB_FUNC(void, vgSetfv, VGParamType, VGint, const VGfloat *);
LIB_FUNC(void, vgSetGlyphToPath, VGFont, VGuint, VGPath, VGboolean,
	const VGfloat [2], const VGfloat [2]);
LIB_FUNC(void, vgSeti, VGParamType, VGint);
LIB_FUNC(void, vgSetPaint, VGPaint, VGbitfield);
LIB_FUNC(void, vgSetParameterfv, VGHandle, VGint, VGint, const VGfloat *);

LIB_FUNC(int, clock_gettime, clockid_t, struct timespec *);

LIB_FUNC(int, XCloseDisplay, Display *);
LIB_FUNC(Window, XCreateWindow, Display *, Window, int, int, unsigned int,
	unsigned int, unsigned int, int, unsigned int, Visual *,
	unsigned long, XSetWindowAttributes *);
LIB_FUNC(int, XDestroyWindow, Display *, Window);
LIB_FUNC(int, XFree, void *);
LIB_FUNC(XVisualInfo *, XGetVisualInfo, Display *, long, XVisualInfo *, int *);
LIB_FUNC(int, XLookupString, XKeyEvent *, char *, int, KeySym *,
	XComposeStatus *);
LIB_FUNC(int, XMapWindow, Display *, Window);
LIB_FUNC(int, XNextEvent, Display *, XEvent *);
LIB_FUNC(Display *, XOpenDisplay, char *);
LIB_FUNC(int, XStoreName, Display *, Window, char *);

extern void libs_init();
extern void libs_stop();

