#include "angel.h"


static struct {
	GLYPH_REND *rend;
	float vscale, hscale; // Ratio of em pixels to 
} mainfont;


void die(char *msg)
{
	printf("error: %s\n",msg);
	exit(EXIT_FAILURE);
}

void load_fonts(const char *execstr)
{
	char *str, *dir;
	GLYPH_FACE *face;
	
	// Figure out where the font file should be
	str = calloc(strlen(execstr) + strlen(MAIN_FONT) + 1,sizeof(char));
	strcpy(str,execstr);
	(dir = strrchr(str,'/')) || (dir = strrchr(str,'\\')) || (dir = str - 1);
	strcpy(dir + 1,MAIN_FONT);
	
	// Load it, hopefully
	face = gk_load_face_from_file(str,0);
	if(!face) die("cannot load font face via GlyphKeeper");
	mainfont.rend = gk_create_renderer(face,0);
	if(!mainfont.rend) die("cannot create GlyphKeeper renderer");
	
	free(str);
}

void print_text(char *str, int x, int y, int w, int h, int r, int g, int b)
{
	int testw, testh;
	
	// Figure out the correct size
	gk_rend_set_size_pixels(mainfont.rend,10,10);
	gk_text_size_utf8(mainfont.rend,str,&testw,&testh);
	gk_rend_set_size_subpixel(mainfont.rend,64*10*w/testw,64*10*h/testh);
	
	// Render it
	gk_rend_set_text_color(mainfont.rend,r,g,b);
	gk_render_line_utf8(screen,mainfont.rend,str,x,y + h);
}

int main(int argc, char **argv)
{
	init_draw();
	load_fonts(argc ? argv[0] : "");
	
	event_loop();
	
	SDL_Quit();
	
	return EXIT_SUCCESS;
}
