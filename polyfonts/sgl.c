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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "sgl.h"

/*--------------------------------------------------------*/
/*
  Useful macros
*/

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a)<0) ? -(a) : (a))
#define sign(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

#define swap(type, a, b) \
{                        \
    type _t_;            \
    _t_ = a;             \
    a = b;               \
    b = _t_;             \
}

#define adjust(v) ((int)(v))

/*--------------------------------------------------------*/

static int initFlag = 0;
static int operation = -1;
static int vertexCount = 0;
static int useGL = 0;

static float red = 0.0;
static float green = 0.0;
static float blue = 0.0;
static float alpha = 0.0;

static Uint32 color = 0;

static SDL_Surface *surface = NULL;

/*--------------------------------------------------------*/

typedef struct
{
  Uint8 set;
  float x;
} edgeRec;

typedef struct
{
  float x, y;
} vertex;

#define numVertsInc (50)
static int  numVerts = 0;
static vertex *verts;

/*-------------------------------------------------*/
/*
  Compute the address of a pixel in any surface
*/

static void *pixelAddress(SDL_Surface *s, int x, int y)
{
  return (void *)(((Uint8 *)s->pixels) + 
                  (y * s->pitch) + 
                  (x * s->format->BytesPerPixel));
}

/*-------------------------------------------------*/
/*
  2D line clipping code
*/

#define mulDiv(a1, a2, a3) (((a1) * (a2)) / (a3))

#define leftEdge(cr)    (cr->x)
#define rightEdge(cr)   ((cr->x + cr->w) - 1)
#define topEdge(cr)     (cr->y)
#define bottomEdge(cr)  ((cr->y + cr->h) - 1)

/*-------------------------------------------------*/
/*
  Do a simple line clip. Return true if there is a
  line left to draw and false if the line was clipped
  completely out of the picture.
*/

static int clipLineXY(SDL_Rect *cr, float *x1, float *y1, float *x2, float *y2)
{
  float dy, dx;

  if ((*x1 < leftEdge(cr) && *x2 < leftEdge(cr)) ||
      (*x1 > rightEdge(cr) && *x2 > rightEdge(cr)) ||
      (*y1 < topEdge(cr) && *y2 < topEdge(cr)) ||
      (*y1 > bottomEdge(cr) && *y2 > bottomEdge(cr)))
  {
    return 0;
  }

  dy = *y2 - *y1;
  dx = *x2 - *x1;

  if (*x1 < leftEdge(cr))
  {
    *y1 = *y1 + mulDiv((leftEdge(cr) - *x1), dy, dx);
    *x1 = leftEdge(cr);
  }
  else if (*x1 > rightEdge(cr))
  {
    *y1 = *y1 + mulDiv((rightEdge(cr) - *x1), dy, dx);
    *x1 = rightEdge(cr);
  }

  if (*y1 < topEdge(cr))
  {
    *x1 = *x1 + mulDiv((topEdge(cr) - *y1), dx, dy);
    *y1 = topEdge(cr);
  }
  else if (*y1 > bottomEdge(cr))
  {
    *x1 = *x1 + mulDiv((bottomEdge(cr) - *y1), dx, dy);
    *y1 = bottomEdge(cr);
  }

  if (*x1 < leftEdge(cr) ||
      *x1 > rightEdge(cr))
  {
    return 0;
  }

  if (*x2 < leftEdge(cr))
  {
    *y2 = *y2 + mulDiv((leftEdge(cr) - *x2), dy, dx);
    *x2 = leftEdge(cr);
  }
  else if (*x2 > rightEdge(cr))
  {
    *y2 = *y2 + mulDiv((rightEdge(cr) - *x2), dy, dx);
    *x2 = rightEdge(cr);
  }

  if (*y2 < topEdge(cr))
  {
    *x2 = *x2 + mulDiv((topEdge(cr) - *y2), dx, dy);
    *y2 = topEdge(cr);
  }
  else if (*y2 > bottomEdge(cr))
  {
    *x2 = *x2 + mulDiv((bottomEdge(cr) - *y2), dx, dy);
    *y2 = bottomEdge(cr);
  }

  if (*x2 < leftEdge(cr) ||
      *x2 > rightEdge(cr))
  {
    return 0;
  }

  return 1;
}

/*-------------------------------------------------*/
/*
  Set a pixel in the surface
*/

static void putPixel8(SDL_Surface *s, int x, int y, Uint32 pixel)
{
  *(Uint8 *)pixelAddress(s, x, y) = pixel;
}

static void putPixel16(SDL_Surface *s, int x, int y, Uint32 pixel)
{
  *(Uint16 *)pixelAddress(s, x, y) = pixel;
}

static void putPixel24(SDL_Surface *s, int x, int y, Uint32 pixel)
{
  Uint8 *p = (Uint8 *)pixelAddress(s, x, y);

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
  pixel <<= 8;
#endif

  memcpy(p, &pixel, 3);
}

static void putPixel32(SDL_Surface *s, int x, int y, Uint32 pixel)
{
  *(Uint32 *)pixelAddress(s, x, y) = pixel;
}

static void putPixel(SDL_Surface *s, int x, int y, Uint32 color)
{
  /*
    first clip to the mode width and height
  */
  if (x < 0 || x >= (s->w) || y < 0 || y >= (s->h))
  {
    return;
  }

  switch (s->format->BytesPerPixel)
  {
  case 1:
    putPixel8(s, x, y, color);
    break;
  case 2:
    putPixel16(s, x, y, color);
    break;
  case 3:
    putPixel24(s, x, y, color);
    break;
  case 4:
    putPixel32(s, x, y, color);
    break;
  }
}

/*-------------------------------------------------*/
/*
  draw clipped lines on a surface
*/

static void drawLine8(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  int d;
  int x;
  int y;
  int ax;
  int ay;
  int sx;
  int sy;
  int dx;
  int dy;

  Uint8 *lineAddr;
  Sint32 yOffset;

  dx = x2 - x1;  
  ax = abs(dx) << 1;  
  sx = sign(dx);

  dy = y2 - y1;  
  ay = abs(dy) << 1;  
  sy = sign(dy);
  yOffset = sy * s->pitch;

  x = x1;
  y = y1;

  lineAddr = ((Uint8 *)s->pixels) + (y * s->pitch);
  if (ax>ay)
  {                      /* x dominant */
    d = ay - (ax >> 1);
    for (;;)
    {
      *(lineAddr + x) = (Uint8)color;

      if (x == x2)
      {
        return;
      }
      if (d>=0)
      {
        y += sy;
        lineAddr += yOffset;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {                      /* y dominant */
    d = ax - (ay >> 1);
    for (;;)
    {
      *(lineAddr + x) = (Uint8)color;

      if (y == y2)
      {
        return;
      }
      if (d>=0) 
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      lineAddr += yOffset;
      d += ax;
    }
  }
}

static void drawLine16(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  int d;
  int x;
  int y;
  int ax;
  int ay;
  int sx;
  int sy;
  int dx;
  int dy;

  Uint8 *lineAddr;
  Sint32 yOffset;

  dx = x2 - x1;  
  ax = abs(dx) << 1;  
  sx = sign(dx);

  dy = y2 - y1;  
  ay = abs(dy) << 1;  
  sy = sign(dy);
  yOffset = sy * s->pitch;

  x = x1;
  y = y1;

  lineAddr = ((Uint8 *)s->pixels) + (y * s->pitch);
  if (ax>ay)
  {                      /* x dominant */
    d = ay - (ax >> 1);
    for (;;)
    {
      *((Uint16 *)(lineAddr + (x << 1))) = (Uint16)color;

      if (x == x2)
      {
        return;
      }
      if (d>=0)
      {
        y += sy;
        lineAddr += yOffset;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {                      /* y dominant */
    d = ax - (ay >> 1);
    for (;;)
    {
      *((Uint16 *)(lineAddr + (x << 1))) = (Uint16)color;

      if (y == y2)
      {
        return;
      }
      if (d>=0) 
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      lineAddr += yOffset;
      d += ax;
    }
  }
}

static void drawLine24(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  int d;
  int x;
  int y;
  int ax;
  int ay;
  int sx;
  int sy;
  int dx;
  int dy;

  Uint8 *lineAddr;
  Sint32 yOffset;

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
  color <<= 8;
#endif

  dx = x2 - x1;  
  ax = abs(dx) << 1;  
  sx = sign(dx);

  dy = y2 - y1;  
  ay = abs(dy) << 1;  
  sy = sign(dy);
  yOffset = sy * s->pitch;

  x = x1;
  y = y1;

  lineAddr = ((Uint8 *)s->pixels) + (y * s->pitch);
  if (ax>ay)
  {                      /* x dominant */
    d = ay - (ax >> 1);
    for (;;)
    {
      Uint8 *p = (lineAddr + (x * 3));
      memcpy(p, &color, 3);

      if (x == x2)
      {
        return;
      }
      if (d>=0)
      {
        y += sy;
        lineAddr += yOffset;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {                      /* y dominant */
    d = ax - (ay >> 1);
    for (;;)
    {
      Uint8 *p = (lineAddr + (x * 3));
      memcpy(p, &color, 3);

      if (y == y2)
      {
        return;
      }
      if (d>=0) 
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      lineAddr += yOffset;
      d += ax;
    }
  }
}

static void drawLine32(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  int d;
  int x;
  int y;
  int ax;
  int ay;
  int sx;
  int sy;
  int dx;
  int dy;

  Uint8 *lineAddr;
  Sint32 yOffset;

  dx = x2 - x1;  
  ax = abs(dx) << 1;  
  sx = sign(dx);

  dy = y2 - y1;  
  ay = abs(dy) << 1;  
  sy = sign(dy);
  yOffset = sy * s->pitch;

  x = x1;
  y = y1;

  lineAddr = ((Uint8 *)s->pixels) + (y * s->pitch);
  if (ax>ay)
  {                      /* x dominant */
    d = ay - (ax >> 1);
    for (;;)
    {
      *((Uint32 *)(lineAddr + (x << 2))) = (Uint32)color;

      if (x == x2)
      {
        return;
      }
      if (d>=0)
      {
        y += sy;
        lineAddr += yOffset;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {                      /* y dominant */
    d = ax - (ay >> 1);
    for (;;)
    {
      *((Uint32 *)(lineAddr + (x << 2))) = (Uint32)color;

      if (y == y2)
      {
        return;
      }
      if (d>=0) 
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      lineAddr += yOffset;
      d += ax;
    }
  }
}

static void drawLine(SDL_Surface *s, float x1, float y1, float x2, float y2, Uint32 color)
{
  SDL_Rect cr;

  x1 = adjust(x1);
  y1 = adjust(y1);

  x2 = adjust(x2);
  y2 = adjust(y2);

  SDL_GetClipRect(s, &cr);
  if (0 == clipLineXY(&cr, &x1, &y1, &x2, &y2))
  {
    return;
  }

  switch (s->format->BytesPerPixel)
  {
  case 1:
    drawLine8(s, x1, y1, x2, y2, color);
    break;
  case 2:
    drawLine16(s, x1, y1, x2, y2, color);
    break;
  case 3:
    drawLine24(s, x1, y1, x2, y2, color);
    break;
  case 4:
    drawLine32(s, x1, y1, x2, y2, color);
    break;
  }
}

/*-------------------------------------------------*/
/*
  Edge list routines
*/

edgeRec *edgeList = NULL;

/*
  make sure that the edge list is at least as long
  as the surface is high.
*/

static void initEdgeList(SDL_Surface *s)
{
  static int height = -1;
  int size = -1;

  if (s->h > height)
  {
    if (NULL != edgeList)
    {
      free(edgeList);
    }

    size = s->h * sizeof(edgeRec);
    edgeList = malloc(size);
    if (NULL == edgeList)
    {
      printf("malloc failed\n");
      exit(1);
    }

    height = s->h;
    memset(edgeList, 0, size);
  }
}

/*-------------------------------------------------*/
/*
  Fill memory with a 24 bit pattern
*/

static void memset3(Uint8 *dst, Uint32 val, int len)
{
  int loLen = len & 0x7;
  int hiLen = len - loLen;
  Uint8 *lim = dst + (len * 3);

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
  val <<= 8;
#endif

  switch(loLen)
  {
  case 7:
    memcpy(dst, &val, 3);
    dst += 3;
  case 6:
    memcpy(dst, &val, 3);
    dst += 3;
  case 5:
    memcpy(dst, &val, 3);
    dst += 3;
  case 4:
    memcpy(dst, &val, 3);
    dst += 3;
  case 3:
    memcpy(dst, &val, 3);
    dst += 3;
  case 2:
    memcpy(dst, &val, 3);
    dst += 3;
  case 1:
    memcpy(dst, &val, 3);
    dst += 3;
  }

  if (hiLen)
  {
    do
    {
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
      memcpy(dst, &val, 3);
      dst += 3;
    } while (dst < lim);
  }
}

/*-------------------------------------------------*/
/*
  Fill memory with a 32 bit pattern
*/

static void memset4(Uint32 *dst, Uint32 val, int len)
{
  int loLen = len & 0x7;
  int hiLen = len - loLen;
  Uint32 *lim = dst + len;

  switch(loLen)
  {
  case 7:
    *dst++ = val;
  case 6:
    *dst++ = val;
  case 5:
    *dst++ = val;
  case 4:
    *dst++ = val;
  case 3:
    *dst++ = val;
  case 2:
    *dst++ = val;
  case 1:
    *dst++ = val;
  }

  if (hiLen)
  {
    do
    {
      dst[0] = val;
      dst[1] = val;
      dst[2] = val;
      dst[3] = val;
      dst[4] = val;
      dst[5] = val;
      dst[6] = val;
      dst[7] = val;
      dst += 8;
    } while (dst < lim);
  }
}

/*-------------------------------------------------*/
/*
  Fill a span in a surface, with no clipping
  this routine is used by other routines to
  render solid colors.
*/

static void fillSolidSpan8(SDL_Surface *s, int x, int y, int dx, Uint32 pixel)
{
  void *p = pixelAddress(s, x, y);
  memset(p, pixel, dx);
}

static void fillSolidSpan16(SDL_Surface *s, int x, int y, int dx, Uint32 pixel)
{
  Uint16 *p = (Uint16 *)pixelAddress(s, x, y);
  Uint32 c2 = pixel | (pixel << 16);

  if (((Uint32)p) & 0x3)
  {
    p[0] = pixel;
    p++;
    dx--;
  }

  if (dx > 0)
  {
    memset4((Uint32 *)p, c2, (dx >> 1));

    if (dx & 0x1)
    {
      p[dx - 1] = pixel;
    }
  }
}

static void fillSolidSpan24(SDL_Surface *s, int x, int y, int dx, Uint32 pixel)
{
  Uint8 *p = (Uint8 *)pixelAddress(s, x, y);
  memset3(p, pixel, dx);
}

static void fillSolidSpan32(SDL_Surface *s, int x, int y, int dx, Uint32 pixel)
{
  void *p = pixelAddress(s, x, y);
  memset4(p, pixel, dx);
}

static void fillSolidSpan(SDL_Surface *s, int x, int y, int dx, Uint32 pixel)
{
  switch (s->format->BytesPerPixel)
  {
  case 1:
    fillSolidSpan8(s, x, y, dx, pixel);
    break;
  case 2:
    fillSolidSpan16(s, x, y, dx, pixel);
    break;
  case 3:
    fillSolidSpan24(s, x, y, dx, pixel);
    break;
  case 4:
    fillSolidSpan32(s, x, y, dx, pixel);
    break;
  }
}

/*-------------------------------------------------*/
/*
  Simple 2D polygon clipping code
*/

/*-------------------------------------------------*/
/*
  use the Sutherland - Hodgman clipping algorithm 
  in screen space.
//
  derived from public domain code
*/

#define SIDES (4)

/*-------------------------------------------------*/

static int first[SIDES];
static int nout, noutmax;
static vertex f[SIDES], s[SIDES];
static vertex cpt;

static int intersectXY(SDL_Rect *cr, int side, vertex *pt1, vertex *pt2, vertex *cpt)
{
  float wc = 0.0, wc1 = 0.0, wc2 = 0.0;

  switch (side)
  {
  case 0:        /* x - left */
    wc1 = leftEdge(cr) - pt1->x;
    wc2 = leftEdge(cr) - pt2->x;
    break;

  case 1:        /* x - right */
    wc1 = rightEdge(cr) - pt1->x;
    wc2 = rightEdge(cr) - pt2->x;
    break;

  case 2:        /* y - bottom */
    wc1 = bottomEdge(cr) - pt1->y;
    wc2 = bottomEdge(cr) - pt2->y;
    break;

  case 3:        /* y - top */
    wc1 = topEdge(cr) - pt1->y;
    wc2 = topEdge(cr) - pt2->y;
    break;

  default:;
    /* dbprintf("intersectXY: ridiculous side value"); */
  }

  /*
    if they are opposite in sign then the edge crosses
  */

  if (((wc1 < 0) && (wc2 > 0)) || ((wc1 > 0) && (wc2 < 0)))
  {
    wc = wc1 - wc2;
    cpt->x = pt1->x + mulDiv(wc1, (pt2->x - pt1->x), wc);
    cpt->y = pt1->y + mulDiv(wc1, (pt2->y - pt1->y), wc);

    return 1;
  }

  return 0;
}

/*-------------------------------------------------*/

static int visiblePointXY(SDL_Rect *cr, int side, vertex *pt)
{
  switch (side)
  {
  case 0:     /* left */
    return pt->x >= leftEdge(cr);
    break;

  case 1:     /* right */
    return pt->x <= rightEdge(cr);
    break;

  case 2:     /* bottom */
    return pt->y <= bottomEdge(cr);
    break;

  case 3:     /* top */
    return pt->y >= topEdge(cr);
    break;

  default:;
    /* dbprintf("visiblePointXY: ridiculous side value"); */
  }

  return 0;
}

/*-------------------------------------------------*/

static void shClipXY(SDL_Rect *cr, int side, vertex *pt, vertex *outpts)
{
  vertex p;

 recur:
  if (side >= SIDES)
  {
    if (nout < noutmax)
    {
      outpts[nout] = *pt;
      nout++;
    }
    else
    {
      /* dbprintf("output points overflow\n"); */
    }
  }
  else
  {
    p = *pt;
    if (first[side])
    {
      first[side] = 0;
      f[side] = p;
    }
    else if (intersectXY(cr, side, &p, &s[side], &cpt))
    {
      shClipXY(cr, side + 1, &cpt, outpts);
    }

    s[side] = p;
    if (visiblePointXY(cr, side, &s[side])) 
    {
      /*            shClipXY(side + 1, &s[side], outpts);
      //
        Get rid of the tail recursion
      */
      pt = &s[side];
      side++;
      goto recur; /* tail recursion */
    }
  }
}

/*-------------------------------------------------*/

static void shCloseXY(SDL_Rect *cr, int side, vertex *outpts)
{
  if (side < SIDES)
  {
    if (intersectXY(cr, side, &f[side], &s[side], &cpt))
    {
      shClipXY(cr, side + 1, &cpt, outpts);
    }

    shCloseXY(cr, side + 1, outpts);

    first[side] = 1;
  }
}

/*-------------------------------------------------*/

static int trivialAcceptXY(SDL_Rect *cr, int npts, vertex *p)
{
  int i;

  for (i = 0; i < npts; i++)
  {
    if ((p[i].x < leftEdge(cr)) ||
        (p[i].x > rightEdge(cr)) ||
        (p[i].y < topEdge(cr)) ||
        (p[i].y > bottomEdge(cr)))
    {
      return 0;
    }
  }

  return 1;
}

/*-------------------------------------------------*/

static int clipPolyXY(SDL_Rect *cr, int npts, vertex *inpts, int maxpts, vertex *outpts)
{
  int i;

  nout = 0;
  noutmax = maxpts;

  /*
    First try trivial accept/reject
  */

  if (trivialAcceptXY(cr, npts, inpts))
  {
    for (i = 0; i < npts; i++)
    {
      outpts[i] = inpts[i];
    }
    return npts;
  }

  /*
    Now do full clipping
  */

  for (i = 0; i < SIDES; i++)
  {
    first[i] = 1;
  }

  for (i = 0; i < npts; i++)
  {
    shClipXY(cr, 0, &inpts[i], outpts);
  }

  shCloseXY(cr, 0, outpts);

  if (nout < 3)
  {
    return 0;
  }

  return nout;
}

/*-------------------------------------------------*/
/*
  Draw a solid filled clipped 2D polygon
*/

static void testSolidPolyEdge(SDL_Surface *s, int x, int y, Uint32 color)
{
  if (edgeList[y].set)
  {
    int lx = min(x, edgeList[y].x);
    int rx = max(x, edgeList[y].x);
    fillSolidSpan(s, lx, y, rx - lx, color);
    edgeList[y].set = 0;
  }
  else
  {
    edgeList[y].set = 1;
    edgeList[y].x = x;
  }
}

static void drawSolidPolyEdge(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  Sint32 x;
  Sint32 fx;
  Sint32 y;
  Sint32 dx;
  Sint32 fdx;
  Sint32 dy;

  if (y1 == y2)
  {
    return;
  }

  if (y2 < y1)
  {
    swap(Sint32, x1, x2);
    swap(Sint32, y1, y2);
  }

  dx = x2 - x1;  
  dy = y2 - y1;  
  fdx = (dx << 10) / dy;
  
  fx = x1 << 10;
  y = y1;

  do
  {
    fx += fdx;
    x = fx >> 10;
    y++;
    testSolidPolyEdge(s, x, y, color);
  } while (y < y2);

}

static void drawSolidPoly(SDL_Surface *s, int npts, vertex *pnts, Uint32 color)
{
  int i;
  int numPnts = 0;
  int maxPnts = npts * 3;
  vertex *points = malloc(maxPnts * sizeof(vertex));
  SDL_Rect cr; 

  for (i = 0; i < npts; i++)
  {
    pnts[i].x = adjust(pnts[i].x);
    pnts[i].y = adjust(pnts[i].y);
  }

  initEdgeList(s);

  SDL_GetClipRect(s, &cr);
  numPnts = clipPolyXY(&cr, npts, pnts, maxPnts, points);

  if (numPnts >= 3)
  {
    for (i = 0; i < (numPnts - 1); i++)
    {
      drawSolidPolyEdge(s, 
                        points[i].x, points[i].y, 
                        points[i + 1].x, points[i + 1].y, 
                        color);
    }
    drawSolidPolyEdge(s, 
                      points[numPnts - 1].x, points[numPnts - 1].y, 
                      points[0].x, points[0].y, 
                      color);
  }

  free(points);
}

/*--------------------------------------------------------*/

static void points()
{
  int i = 0;

  for (i = 0; i < vertexCount; i++)
  {
    putPixel(surface, (int)verts[i].x, (int)verts[i].y, color);
  }
}

/*--------------------------------------------------------*/

static void lines()
{
  int i = 0;

  if (vertexCount >= 2)
  {
    vertexCount -= (vertexCount % 2);
    for (i = 0; i < vertexCount; i += 2)
    {
      drawLine(surface, 
               (int)verts[i + 0].x, (int)verts[i + 0].y, 
               (int)verts[i + 1].x, (int)verts[i + 1].y, 
               color);
    }
  }
}

/*--------------------------------------------------------*/

static void lineLoop()
{
  int i = 0;

  if (vertexCount >= 2)
  {
    for (i = 0; i < (vertexCount - 1); i++)
    {
      drawLine(surface, 
               (int)verts[i + 0].x, (int)verts[i + 0].y, 
               (int)verts[i + 1].x, (int)verts[i + 1].y, 
               color);
    }

    drawLine(surface, 
             (int)verts[vertexCount - 1].x, (int)verts[vertexCount - 1].y, 
             (int)verts[0].x, (int)verts[0].y, 
             color);
  }
}

/*--------------------------------------------------------*/

static void lineStrip()
{
  int i = 0;

  if (vertexCount >= 2)
  {
    for (i = 0; i < (vertexCount - 1); i++)
    {
      drawLine(surface, 
               (int)verts[i + 0].x, (int)verts[i + 0].y, 
               (int)verts[i + 1].x, (int)verts[i + 1].y, 
               color);
    }
  }
}

/*--------------------------------------------------------*/

static void triangles()
{
  int i = 0;

  if (vertexCount >= 3)
  {
    vertexCount -= (vertexCount % 3);
    for (i = 0; i < vertexCount; i += 3)
    {
      drawSolidPoly(surface, 3, &verts[i], color);
    }
  }
}

/*--------------------------------------------------------*/

static void triangleStrip()
{
  int i = 0;

  if (vertexCount >= 3)
  {
    for (i = 0; i < (vertexCount - 2); i++)
    {
      drawSolidPoly(surface, 3, &verts[i], color);
    }
  }
}

/*--------------------------------------------------------*/

static void triangleFan()
{
  vertex v[3];
  int i = 0;

  if (vertexCount >= 3)
  {
    v[0] = verts[0];
    for (i = 1; i < (vertexCount - 1); i++)
    {
      v[1] = verts[i + 0];
      v[2] = verts[i + 1];
      drawSolidPoly(surface, 3, &v[0], color);
    }
  }
}

/*--------------------------------------------------------*/
/*
  The rather odd looking code in quads(),
  quadStrip(), polygon() is there so that we get
  the same kind of butterfly effect for bad vertex
  order that OpenGL gives us. You have to
  duplicate the bad with the good.
 */

static void quads()
{
  int i = 0;
  vertex v[3];

  if (vertexCount >= 4)
  {
    vertexCount -= (vertexCount % 4);
    for (i = 0; i < vertexCount; i += 4)
    {
      v[0] = verts[i + 0];

      v[1] = verts[i + 1];
      v[2] = verts[i + 2];
      drawSolidPoly(surface, 3, &v[0], color);

      v[1] = verts[i + 2];
      v[2] = verts[i + 3];
      drawSolidPoly(surface, 3, &v[0], color);
    }
  }
}

/*--------------------------------------------------------*/

static void quadStrip()
{
  int i = 0;
  vertex v[3];

  if (vertexCount >= 4)
  {
    vertexCount -= (vertexCount % 2);
    for (i = 0; i < (vertexCount - 2); i += 2)
    {
      v[0] = verts[i + 0];
      v[1] = verts[i + 1];
      v[2] = verts[i + 2];
      drawSolidPoly(surface, 3, &v[0], color);
      v[0] = verts[i + 1];
      v[1] = verts[i + 2];
      v[2] = verts[i + 3];
      drawSolidPoly(surface, 3, &v[0], color);
    }
  }
}

/*--------------------------------------------------------*/
/*
 */
static void polygon()
{
  triangleFan();
  /*
  if (vertexCount >= 3)
  {
    drawSolidPoly(surface, vertexCount, &verts[0], color);
  }
  */
}

/*--------------------------------------------------------*/

static void sglInit()
{
  if (!initFlag)
  {
    verts = malloc (sizeof(vertex) * numVertsInc);
    if (NULL == verts)
    {
      printf("Out Of Memory\n");
      exit(1);
    }

    numVerts = numVertsInc;

    useGL = 0;
    operation = -1;
    vertexCount = 0;

    initFlag = 1;
  }
}

/*--------------------------------------------------------*/

void sglBegin(SDL_Surface *s, int p)
{
  sglInit();

  operation = p;
  surface = s;
  vertexCount = 0;
  useGL = s->flags & SDL_OPENGL;

  if (useGL)
  {
#ifdef USE_GL
    glColor4f(red, green, blue, alpha);
    glBegin(operation);
#else
    printf("USE_GL is not defined in sgl.h\n");
    exit(1);
#endif
  }
}

/*--------------------------------------------------------*/

void sglEnd()
{
  sglInit();

  if (-1 == operation)
  {
    return;
  }

  if (useGL)
  {
#ifdef USE_GL
    glEnd();
#else
    printf("USE_GL is not defined in sgl.h\n");
    exit(1);
#endif
  }
  else
  {
    color = SDL_MapRGBA(surface->format,
                        red * 255,
                        green * 255,
                        blue * 255,
                        alpha * 255);
    switch(operation)
    {
    case GL_POINTS:
      points();
      break;

    case GL_LINES:
      lines();
      break;

    case GL_LINE_LOOP:
      lineLoop();
      break;

    case GL_LINE_STRIP:
      lineStrip();
      break;

    case GL_TRIANGLES:
      triangles();
      break;

    case GL_TRIANGLE_STRIP:
      triangleStrip();
      break;

    case GL_TRIANGLE_FAN:
      triangleFan();
      break;

    case GL_QUADS:
      quads();
      break;

    case GL_QUAD_STRIP:
      quadStrip();
      break;

    case GL_POLYGON:
      polygon();
      break;
    }
  }

  operation = -1;
}

/*--------------------------------------------------------*/

void sglVertex2f(float x, float y)
{
  sglInit();

  if (-1 == operation)
  {
    return;
  }

  if (useGL)
  {
#ifdef USE_GL
    glVertex2f(x, y);
#else
    printf("USE_GL is not defined in sgl.h\n");
    exit(1);
#endif
  }
  else
  {
    if (vertexCount >= numVerts)
    {
      numVerts += numVertsInc;
      verts = realloc(verts, sizeof(vertex) * numVerts);
      if (NULL == verts)
      {
        printf("Out Of Memory\n");
        exit(1);
      }
    }

    verts[vertexCount].x = x;
    verts[vertexCount].y = y;

    vertexCount++;
  }
}

/*--------------------------------------------------------*/

void sglColor3f(float r, float g, float b)
{
  sglInit();

  red   = min(1.0, max(r, 0.0));
  green = min(1.0, max(g, 0.0));
  blue  = min(1.0, max(b, 0.0));
  alpha = 0.0;
}

/*--------------------------------------------------------*/

void sglColor4f(float r, float g, float b, float a)
{
  sglInit();

  red   = min(1.0, max(r, 0.0));
  green = min(1.0, max(g, 0.0));
  blue  = min(1.0, max(b, 0.0));
  alpha = min(1.0, max(a, 0.0));
}

/*--------------------------------------------------------*/

void sglSwapBuffers(SDL_Surface *s)
{
  sglInit();

  if (0 != (s->flags & SDL_OPENGL))
  {
#ifdef USE_GL
    SDL_GL_SwapBuffers();
#else
    printf("USE_GL is not defined in sgl.h\n");
    exit(1);
#endif
  }
  else
  {
    SDL_Flip(s);
  }
}

/*--------------------------------------------------------*/

void sglClearBuffer(SDL_Surface *s, 
                    float r, float g, float b, float a,
                    float z)
{
  Uint32 fill = 0;
#ifdef USE_GL
  float saveColor[4];
  float saveZ;
#endif

  sglInit();

  if (0 != (s->flags & SDL_OPENGL))
  {
#ifdef USE_GL
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &saveColor[0]);
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &saveZ);

    glClearDepth(z);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearDepth(saveZ);
    glClearColor(saveColor[0],
                 saveColor[1],
                 saveColor[2],
                 saveColor[3]);
#else
    printf("USE_GL is not defined in sgl.h\n");
    exit(1);
#endif
  }
  else
  {
    fill = SDL_MapRGBA(s->format,
                       r * 255,
                       g * 255,
                       b * 255,
                       a * 255);
    SDL_FillRect(s, NULL, fill);
  }
}

