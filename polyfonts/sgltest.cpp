#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "sgl.h"

#include "polyfonts.h"
#include "fonts/pfSRoman.h"
#include "fonts/pfPSans8.h"
#include "fonts/pfOSans8.h"
#include "fonts/pfPSansBold8.h"

//-------------------------------------------------

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a)<0) ? -(a) : (a))
#define sign(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

#define scaleStart (20)
#define scaleEnd   (400)

//-------------------------------------------------

SDL_Surface *screen = NULL;

int sw = 800;
int sh = 600;

int scale = scaleStart;
float skew = 0.0;
int TorG = 1;
int weight = 1;

char *glPrims[] = 
  {
    "GL_POINTS",
    "GL_LINES",
    "GL_LINE_LOOP",
    "GL_LINE_STRIP",
    "GL_TRIANGLES",
    "GL_TRIANGLE_STRIP",
    "GL_TRIANGLE_FAN",
    "GL_QUADS",
    "GL_QUAD_STRIP",
    "GL_POLYGON",
  };

//-------------------------------------------------

int gnum = 0;
int gcol = 0;

typedef struct
{
  float r, g, b;
} color;

color colors[] = 
  {
    {0.0, 0.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 1.0},
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 1.0},
    {1.0, 1.0, 0.0},
  };

int numColors = (sizeof(colors) / sizeof(color));

typedef struct
{
  float x, y;
} vertex;

/*
  vertex points[] = 
  {
  { 10.0,  10.0},
  {100.0,  10.0},
  {100.0, 100.0},
  { 10.0, 100.0},
  };
*/

vertex points[] = 
  {
    {140.609741, 437.484741},
    {109.359741, 421.859741},
    {78.109741, 390.609741},
    {62.484741, 359.359741},
    {46.859741, 312.484741},
    {46.859741, 234.359741},
    {62.484741, 187.484741},
    {78.109741, 156.234741},
    {109.359741, 124.984741},
    {140.609741, 109.359741},
    {203.109741, 109.359741},
    {234.359741, 124.984741},
    {265.609741, 156.234741},
    {281.234741, 187.484741},
    {296.859741, 234.359741},
    {296.859741, 312.484741},
    {281.234741, 359.359741},
    {265.609741, 390.609741},
    {234.359741, 421.859741},
    {203.109741, 437.484741},
    {140.609741, 437.484741},
  };

int numPoints = (sizeof(points) / sizeof(vertex));

void graphics()
{
  int i = 0;
  int k = 0;

  printf("grahic test %s\n", glPrims[gnum]);
  sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);
  gcol = (gcol + 1) % numColors;

  switch(gnum)
  {
  case 0:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_POINTS);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 1:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_LINES);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 2:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_LINE_LOOP);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 3:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_LINE_STRIP);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 4:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_TRIANGLES);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 5:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_TRIANGLE_STRIP);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 6:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_TRIANGLE_FAN);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 7:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_QUADS);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 8:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_QUAD_STRIP);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum++;
    break;

  case 9:

    for (k = numPoints; k > 0; k--)
    {
      gcol = (gcol + 1) % numColors;
      sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);

      sglBegin(screen, GL_POLYGON);
      for (i = 0; i < k; i++)
      {
        sglVertex2f(points[i].x, points[i].y);
      }
      sglEnd();
    }

    gnum = 0;
    break;

  default:
    gnum = 0;
  }
}

//-------------------------------------------------

void drawCharBox(wchar_t c, float x, float y)
{
  float minx, miny, maxx, maxy;

  pfGetCharBBox(c, &minx, &miny, &maxx, &maxy);
  minx = x + minx;
  maxx = x + maxx;
  miny = y - miny;
  maxy = y - maxy;

  sglColor3f(0.0, 0.0, 0.0);
  //sglBegin(screen, GL_QUADS);
  sglBegin(screen, GL_LINE_LOOP);
  sglVertex2f(minx, miny);
  sglVertex2f(minx, maxy);
  sglVertex2f(maxx, maxy);
  sglVertex2f(maxx, miny);
  sglEnd();

  sglColor3f(0.0, 1.0, 0.0);
  sglBegin(screen, GL_LINES);
  sglVertex2f(minx, y);
  sglVertex2f(maxx, y);
  sglEnd();
}

//-------------------------------------------------

void drawGrid(int o, float r, float g, float b)
{
  int i;

  sglColor3f(r, g, b);

  sglBegin(screen, GL_LINES);
  for (i = o; i < sh; i += o)
  {
    sglVertex2f(0, i);
    sglVertex2f(sw, i);
  }

  for (i = o; i < sw; i += o)
  {
    sglVertex2f(i, 0);
    sglVertex2f(i, sh);
  }
  sglEnd();
}

//-------------------------------------------------

void text()
{
  int glyph = 0;
  float minx, miny, maxx, maxy;
  float x = 0;
  float y = 0;
  int i = 0;

  //drawGrid(100, 0.0, 0.1, 0.0);
  //pfSetFont(&pfPSans8);
  //pfSetFont(&pfPSansBold8);
  //pfSetFont(&pfOSans8);
  //pfSetFont(&pfSRoman);

  pfSetScale(scale);
  pfSetSkew(skew);
  pfSetWeight(weight);

  /*
    Print Font Metrics
   */
  printf("Skew = %f\n", skew);
  printf("Font Name    =%s\n", pfGetFontName());
  printf("Font Height  =%f\n", pfGetFontHeight());
  printf("Font Width   =%f\n", pfGetFontWidth());
  printf("Font Ascent  =%f\n", pfGetFontAscent());
  printf("Font Descent =%f\n", pfGetFontDescent());
  pfGetFontBBox(&minx, &miny, &maxx, &maxy);
  printf("Font BBox minx=%f, miny=%f, maxx=%f, maxy=%f\n", 
         minx, miny, maxx, maxy);

  /*
    Print out the whole font.
   */
  x = 0;
  y = pfGetFontAscent();
  pfSetPosition(x, y);

  for (int i = 0; i < pfGetFontNumGlyphs(); i++)
  {
    glyph = pfGetChar(i);

    if ((x + pfGetCharAdvance(glyph)) > sw)
    {
      x = 0;
      y += pfGetFontHeight();
      pfSetPosition(x, y);
    }

    //drawCharBox(glyph, x, y);
    sglColor3f(1.0, 0.0, 0.0);
    pfDrawChar(screen, glyph);

    printf("Char Name    =\"%c\"\n", glyph);
    printf("Char Height  =%f\n", pfGetCharHeight(glyph));
    printf("Char Width   =%f\n", pfGetCharWidth(glyph));
    printf("Char Ascent  =%f\n", pfGetCharAscent(glyph));
    printf("Char Descent =%f\n", pfGetCharDescent(glyph));
    printf("Char Advance =%f\n", pfGetCharAdvance(glyph));
    pfGetCharBBox(glyph, &minx, &miny, &maxx, &maxy);
    printf("Char BBox minx=%f, miny=%f, maxx=%f, maxy=%f\n", 
           minx, miny, maxx, maxy);

    pfGetPosition(&x, &y);
  }

  /*
    Draw Text scaled, skewed and rotated, in different ways
   */

  //for (i = 0; i < 360; i += 30)
  for (i = 360; i >= 0; i -= 30)
  {
    sglColor3f(colors[gcol].r, colors[gcol].g, colors[gcol].b);
    gcol = (gcol + 1) % numColors;

    pfSetAngleD((float)i);
    pfSetPosition(sw / 2, sh / 2);
    pfSetScaleBox("Hello World!", i, i / 3.0);
    pfDrawString(screen, "Hello World!");
  }
  pfSetAngleD(0.0);

  scale = max(scaleStart, (scale + 1) % scaleEnd);

  skew += 0.1;
  if (1.0 < skew)
  {
    skew = -1.0;
  }

  weight += 1;
  if (weight > 9)
  {
    weight = 1;
  }
}

//-------------------------------------------------

void test(char *first)
{
  char title[200];

  if (TorG)
  {
    sprintf(title, "%s%s", first, "Text");
    SDL_WM_SetCaption(title, title);

    text();
  }
  else
  {
    sprintf(title, "%s%s", first, "Graphics");
    SDL_WM_SetCaption(title, title);

    graphics();
  }
}

//-------------------------------------------------

void push2D()
{
  SDL_Surface *screen = SDL_GetVideoSurface();

  /* Note, there may be other things you need to change,
     depending on how you have your OpenGL state set up.
  */
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  /* This allows alpha blending of 2D textures with the scene */
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, screen->w, screen->h);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  //glEnable(GL_POLYGON_SMOOTH);
  //glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);

}

//-------------------------------------------------

void pop2D()
{
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();
}

//-------------------------------------------------

void errorExit(char *msg)
{
  printf("%s\n", msg);
  SDL_Quit();
  exit(1);
}

//-------------------------------------------------

void sdltest()
{
  bool done = false;
  SDL_Event event;
  Uint32 white;

  if (-1 == SDL_Init((SDL_INIT_VIDEO |
                      SDL_INIT_EVENTTHREAD)))
  {
    errorExit("Can't initialize SDL");
  }

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  screen = SDL_SetVideoMode(sw, sh, 0, SDL_ANYFORMAT | SDL_SWSURFACE);

  if (NULL == screen)
  {
    errorExit("Can't set video mode");
  }

  white = SDL_MapRGB(screen->format, 0xff, 0xff, 0xff);

  sglClearBuffer(screen, 1.0, 1.0, 1.0, 0.0, 1.0);
  sglSwapBuffers(screen);
  sglClearBuffer(screen, 1.0, 1.0, 1.0, 0.0, 1.0);
  sglSwapBuffers(screen);

  while ((!done) && SDL_WaitEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      done = true;
      break;

    case SDL_KEYDOWN:
      switch(event.key.keysym.sym)
      {
      case SDLK_q:
      case SDLK_ESCAPE:
        done = true;
        break;

      case SDLK_t:
        TorG = 1;

        sglClearBuffer(screen, 1.0, 1.0, 1.0, 0.0, 1.0);

        test("SGL - ");

        sglSwapBuffers(screen);
        break;

      case SDLK_g:
        TorG = 0;

        sglClearBuffer(screen, 1.0, 1.0, 1.0, 0.0, 1.0);

        test("SGL - ");

        sglSwapBuffers(screen);
        break;

      case SDLK_SPACE:
        sglClearBuffer(screen, 1.0, 1.0, 1.0, 0.0, 1.0);

        test("SGL - ");

        sglSwapBuffers(screen);
        break;

      default:
        break;
      }
      break;
    }
  }
    
  SDL_Quit();
}

//-------------------------------------------------

void gltest()
{
  bool done = false;
  SDL_Event event;
  float r = 1.0, g = 1.0, b = 1.0;

  if (-1 == SDL_Init((SDL_INIT_VIDEO |
                      SDL_INIT_EVENTTHREAD)))
  {
    errorExit("Can't initialize SDL");
  }

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  screen = SDL_SetVideoMode(sw, sh, 0, SDL_OPENGL);

  if (NULL == screen)
  {
    errorExit("Can't set video mode");
  }

  sglClearBuffer(screen, r, g, b, 0.0, 1.0);
  sglSwapBuffers(screen);
  sglClearBuffer(screen, r, g, b, 0.0, 1.0);
  sglSwapBuffers(screen);

  while ((!done) && SDL_WaitEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      done = true;
      break;

    case SDL_KEYDOWN:
      switch(event.key.keysym.sym)
      {
      case SDLK_q:
      case SDLK_ESCAPE:
        done = true;
        break;

      case SDLK_t:
        TorG = 1;
 
        sglClearBuffer(screen, r, g, b, 0.0, 1.0);
        
        push2D();

        test("OpenGL - ");

        pop2D();
        sglSwapBuffers(screen);
        break;

      case SDLK_g:
        TorG = 0;

        sglClearBuffer(screen, r, g, b, 0.0, 1.0);
        push2D();

        test("OpenGL - ");

        pop2D();
        sglSwapBuffers(screen);
        break;

      case SDLK_SPACE:
        sglClearBuffer(screen, r, g, b, 0.0, 1.0);
        push2D();

        test("OpenGL - ");

        pop2D();
        sglSwapBuffers(screen);
        break;

      default:
        break;
      }
      break;
    }
  }
    
  SDL_Quit();
}

//-------------------------------------------------

static void makepoints()
{
  Uint16 oh[] = {9215,28671,7167,27647,5119,25599,4095,23551,3071,20479,3071,15359,4095,12287,5119,10239,7167,8191,9215,7167,13311,7167,15359,8191,17407,10239,18431,12287,19455,15359,19455,20479,18431,23551,17407,25599,15359,27647,13311,28671,9215,28671,};
#define unfix(value) (((float)(value)) / ((float)pfFixScale) * 500.0)

  Uint32 i;
  for (i = 0; i < (sizeof(oh) / sizeof(Uint16)); i += 2)
  {
    printf("{%f, %f},\n", unfix(oh[i]), unfix(oh[i+1]));
  }
  exit(0);
}

//-------------------------------------------------

int main(int argc, char **argv)
{
  TorG = 1;
  gnum = 0;
  gcol = 0;
  scale = scaleStart;
  skew = 0.0;
  weight = 1;
  sdltest();

  TorG = 1;
  gnum = 0;
  gcol = 0;
  scale = scaleStart;
  skew = 0.0;
  weight = 1;
  gltest();
}
