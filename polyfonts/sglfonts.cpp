#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "SDL.h"
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
  Uint32 black, white;
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

  screen = SDL_SetVideoMode(sw, sh, 0, SDL_ANYFORMAT | SDL_SWSURFACE);
  black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
  white = SDL_MapRGB(screen->format, 0xff, 0xff, 0xff);


  if (NULL == screen)
  {
    errorExit("Can't set video mode");
  }

  SDL_FillRect(screen, NULL, white);
  SDL_Flip(screen);
  SDL_FillRect(screen, NULL, white);
  SDL_Flip(screen);

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
        //pfSetAngleD(315);
        SDL_WM_SetCaption(pfGetFontName(), pfGetFontName());

        x = 0;
        y = pfGetFontAscent();
        pfSetPosition(x, y);

        SDL_FillRect(screen, NULL, white);

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

        SDL_Flip(screen);

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
