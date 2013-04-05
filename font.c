#include <assert.h>
#include <math.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <VG/openvg.h>


int numfonts;
VGFont *fonts;

FT_Library ft;

struct {
	VGPath path;
	VGfloat escapement[2];
} glyphs[0x7F];


static int move_to(const FT_Vector *, void *);
static int line_to(const FT_Vector *, void *);
static int conic_to(const FT_Vector *, const FT_Vector *, void *);
static int cubic_to(const FT_Vector *, const FT_Vector *, const FT_Vector *,
	void *);


void font_init() {
	numfonts = 0;
	fonts = NULL;

	FT_Init_FreeType(&ft);
}

void font_stop() {
	FT_Done_FreeType(ft);
}

int font_load(char *fontfile) {
	int i;
	VGFont font;
	VGPath path;
	FT_Face face;

	const FT_Outline_Funcs funcs = {
		.move_to = move_to,
		.line_to = line_to,
		.conic_to = conic_to,
		.cubic_to = cubic_to,
		.shift = 0,
		.delta = 0
	};

	// Initialize FreeType
	FT_New_Face(ft,fontfile,0,&face);
	FT_Select_Charmap(face,FT_ENCODING_UNICODE);
	FT_Set_Pixel_Sizes(face,100,100);

	// Initialize OpenVG font stuff
	font = vgCreateFont(0xFE - 0x20 + 1);

	// Load printable ASCII characters
	for(i = 0x20; i <= 0x7E; i++) {
		FT_Load_Char(face,i,FT_LOAD_NO_BITMAP);

		if(face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
			printf("character %c does not have an outline\n",
				(char) i);
			break;
		}

		path = vgCreatePath(VG_PATH_FORMAT_STANDARD,VG_PATH_DATATYPE_F,
			1,0,0,face->glyph->outline.n_points,
			VG_PATH_CAPABILITY_ALL);
		FT_Outline_Decompose(&face->glyph->outline,&funcs,&path);

		glyphs[i].escapement[0] = (float) face->glyph->advance.x / 64;
		glyphs[i].escapement[1] = (float) face->glyph->advance.y / 64;

		glyphs[i].path = path;
	}

	fonts = realloc(fonts,++numfonts*sizeof *fonts);
	fonts[numfonts - 1] = font;

	FT_Done_Face(face);

	return numfonts - 1;
}

void font_print(int font, float x, float y, char *text) {
	int i;

	vgTranslate(x,y);

	for(i = 0; i < strlen(text); i++) {
		vgDrawPath(glyphs[(int) text[i]].path,
			VG_FILL_PATH | VG_STROKE_PATH);
		vgTranslate(glyphs[(int) text[i]].escapement[0],
			glyphs[(int) text[i]].escapement[1]);
	}
}

static int move_to(const FT_Vector *to, void *user) {
	VGubyte seg = VG_MOVE_TO;
	VGfloat coords[2] = {
		(float) to->x / 64,
		(float) to->y / 64
	};

	vgAppendPathData(*(VGPath *) user,1,&seg,coords);

	return 0;
}

static int line_to(const FT_Vector *to, void *user) {
	VGubyte seg = VG_LINE_TO;
	VGfloat coords[2] = {
		(float) to->x / 64,
		(float) to->y / 64
	};

	vgAppendPathData(*(VGPath *) user,1,&seg,coords);

	return 0;
}

static int conic_to(const FT_Vector *ctrl, const FT_Vector *to, void *user) {
	VGubyte seg = VG_QUAD_TO;
	VGfloat coords[4] = {
		(float) ctrl->x / 64,
		(float) ctrl->y / 64,
		(float) to->x / 64,
		(float) to->y / 64
	};

	vgAppendPathData(*(VGPath *) user,1,&seg,coords);

	return 0;
}

static int cubic_to(const FT_Vector *ctrl0, const FT_Vector *ctrl1,
		const FT_Vector *to, void *user) {
	VGubyte seg = VG_CUBIC_TO;
	VGfloat coords[6] = {
		(float) ctrl0->x / 64,
		(float) ctrl0->y / 64,
		(float) ctrl1->x / 64,
		(float) ctrl1->y / 64,
		(float) to->x / 64,
		(float) to->y / 64
	};

	vgAppendPathData(*(VGPath *) user,1,&seg,coords);

	return 0;
}

