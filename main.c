#include "angel.h"


static GLYPH_REND *mainfont;


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
	mainfont = gk_create_renderer(face,0);
	if(!mainfont) die("cannot create GlyphKeeper renderer");
	
	free(str);
}

void print_text(char *str, int x, int y, int w, int h, int r, int g, int b)
{
	gk_rend_set_size_subpixel(mainfont,64*w*screenhscale,64*h*screenvscale);
	
	gk_rend_set_text_color(mainfont,r,g,b);
	
	gk_render_line_utf8(screen,mainfont,str,x,y + h);
}

int main(int argc, char **argv)
{
	serial_init();
	data_init();
	
	draw_init();
	load_fonts(argc ? argv[0] : "");
	
	event_loop();
	
	SDL_Quit();
	
	return EXIT_SUCCESS;
}
