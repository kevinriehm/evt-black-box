#include "angel.h"


TTF_Font *mainfont;


void die(char *msg)
{
	printf("error: %s\n",msg);
	exit(EXIT_FAILURE);
}

void load_fonts(const char *execstr)
{
	char *str, *dir;
	
	// Figure out where the font file is
	str = calloc(strlen(execstr) + strlen(MAIN_FONT) + 1,sizeof(char));
	strcpy(str,execstr);
	(dir = strrchr(str,'/')) || (dir = strrchr(str,'\\')) || (dir = str - 1);
	strcpy(dir + 1,MAIN_FONT);
	
	mainfont = TTF_OpenFont(str,30);
	if(!mainfont) die(SDL_GetError());
	
	free(str);
}

void print_text(char *str, int x, int y, int r, int g, int b)
{
	SDL_Rect destrect = {.x = x, .y = y};
	SDL_Color fg = {.r = r, .g = g, .b = b},
		bg = {.r = 0xFF, .g = 0xFF, .b = 0xFF};
	
	SDL_Surface *surface = TTF_RenderText_Shaded(mainfont,str,fg,bg);
	if(!surface) die(TTF_GetError());

	SDL_BlitSurface(surface,NULL,screen,&destrect);
	SDL_FreeSurface(surface);
}

int main(int argc, char **argv)
{
	init_draw();
	TTF_Init();
	load_fonts(argc ? argv[0] : "");
	
	event_loop();
	
	TTF_CloseFont(mainfont);
	
	TTF_Quit();
	SDL_Quit();
	
	return EXIT_SUCCESS;
}
