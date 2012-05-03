#ifndef _SGL_H_
#define _SGL_H_

/*
  SGL is a simle graphics library for use with SDL. It implements a
  tiny subset of GL, just enough for drawing text and GUI
  elements. SGL renders into SDL hardware and software surfaces using
  software rendering code and into GL surfaces using GL. It will draw
  into any kind of SDL surface.

  Copyright (C) 2003 Bob Pendleton

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2.1
  of the License, or (at your option) any later version.
    
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
    
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307 USA

  If you do not wish to comply with the terms of the LGPL please
  contact the author as other terms are available for a fee.
    
  Bob Pendleton
  Bob@Pendleton.com
*/

#include "SDL.h"

/*
 If you define "USE_GL" sdl.c will compile with OpenGL
 support. Otherwise it just supports SDL hardware and software
 buffers. Even if OpenGL support is compiled in, sgl still supports
 SDL hardware and software buffers.
*/

#define USE_GL
#ifdef USE_GL
#include "SDL_opengl.h"
#else

/*
 If USE_GL isn't defined then none of the GL primitives are defined
 either. They must be defined somewhere because sgl.h and
 polyfontdefs.h both require them to compile correctly.
*/

#define GL_POINTS				0x0000
#define GL_LINES				0x0001
#define GL_LINE_LOOP				0x0002
#define GL_LINE_STRIP				0x0003
#define GL_TRIANGLES				0x0004
#define GL_TRIANGLE_STRIP			0x0005
#define GL_TRIANGLE_FAN				0x0006
#define GL_QUADS				0x0007
#define GL_QUAD_STRIP				0x0008
#define GL_POLYGON				0x0009

#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sglBegin(SDL_Surface *s, int prim);
  void sglVertex2f(float x, float y);
  void sglEnd();

  void sglColor3f(float r, float g, float b);
  void sglColor4f(float r, float g, float b, float a);

  void sglSwapBuffers(SDL_Surface *s);
  void sglClearBuffer(SDL_Surface *s, 
                      float r, float g, float b, float a,
                      float z);

#ifdef __cplusplus
}
#endif

#endif
