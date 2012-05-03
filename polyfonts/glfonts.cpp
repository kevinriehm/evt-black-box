#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "sgl.h"

#include "polyfonts.h"
#include "polyfontsALL.h"

//-------------------------------------------------

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a)<0) ? -(a) : (a))
#define sign(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

#define scaleStart      (40)
#define scaleIncrement  (10)
#define scaleEnd       (400)

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

SDL_Surface *screen = NULL;

int sw = 800;
int sh = 600;

bool done = false;

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

void errorExit(char *msg)
{
  printf("%s\n", msg);
  SDL_Quit();
  exit(1);
}

//-------------------------------------------------

int main(int argc, char **argv)
{
  SDL_Event event;
  float r = 1.0, g = 1.0, b = 1.0;
  int numFonts = pfNumFonts;
  int font = 0;
  int glyph = 0;
  float x = 0;
  float y = 0;
  int scale = scaleStart;
  float skew = -1;

  if (-1 == SDL_Init((SDL_INIT_VIDEO |
                      SDL_INIT_EVENTTHREAD)))
  {
    errorExit("Can't initialize SDL");
  }
  atexit(SDL_Quit);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  screen = SDL_SetVideoMode(sw, sh, 0, SDL_OPENGL);

  if (NULL == screen)
  {
    errorExit("Can't set video mode");
  }

  glClearColor(r, g, b, 0.0);
  //glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapBuffers();

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

      case SDLK_SPACE:
        pfSetFont(pfAllFonts[font]);
        pfSetScale(scale);
        //pfSetSkew(skew);
        //pfSetWeight(1);
        //pfSetAngleD(315);
        SDL_WM_SetCaption(pfGetFontName(), pfGetFontName());

        x = 0;
        y = pfGetFontAscent();
        pfSetPosition(x, y);

        glClear(GL_COLOR_BUFFER_BIT);

        push2D();

        for (int i = 0; i < pfGetFontNumGlyphs(); i++)
        {
          glyph = pfGetChar(i);

          if ((x + pfGetCharAdvance(glyph)) > sw)
          {
            x = 0;
            y += pfGetFontHeight();
            pfSetPosition(x, y);
          }

          pfGetPosition(&x, &y);
          //pfSetCenter(0);
          //drawCharBox(glyph, x, y);
          sglColor3f(1.0, 0.0, 0.0);
          pfDrawChar(screen, glyph);

          //pfSetPosition(x, y);
          //pfSetCenter(1);
          //pfSetColor(0.0, 0.0, 1.0);
          //pfDrawChar(screen, glyph);

          pfGetPosition(&x, &y);
        }

        pop2D();
        SDL_GL_SwapBuffers();

        font = (font + 1) % numFonts;
        if (0 == font)
        {
          scale = max(scaleStart, (scale + scaleIncrement) % scaleEnd);

          skew += 0.1;
          if (1.0 < skew)
          {
            skew = -1;
          }
        }
        break;

      default:
        break;
      }
      break;
    }
  }
    
  exit(0);
}
