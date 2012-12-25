#include <stdlib.h>

#include <EGL/egl.h>
#include <VG/openvg.h>

#include "display.h"
#include "linked_list.h"
#include "main.h"
#include "pil.h"
#include "scheduler.h"


#define GUI_REFRESH_MS 1000


typedef struct {
	VGPath path;
	VGPaint fill, edge;
} gui_obj_t;


gui_obj_t *guiroot;


static VGPaint convert_pil_paint(pil_paint_t *pilpaint) {
	VGfloat rgba[4];
	VGPaint vgpaint = vgCreatePaint();

	switch(pilpaint->type) {
	case PIL_COLOR: // RGB[A]
		rgba[0] = pilpaint->data.color.r;
		rgba[1] = pilpaint->data.color.g;
		rgba[2] = pilpaint->data.color.b;
		rgba[3] = pilpaint->data.color.a;

		vgSetParameterfv(vgpaint,VG_PAINT_COLOR,4,rgba);
		break;

	case PIL_UNKNOWN_PAINT:
	default:
		break;
	}

	return vgpaint;
}

static void append_pil_seg(VGPath path, pil_seg_t *seg) {
	int i;
	VGubyte pathseg;
	VGfloat datafv[2];

	switch(seg->type) {
	case PIL_LINE:
		for(i = 0; i < seg->data.line.numpoints; i++) {
			pathseg = VG_LINE_TO;
			datafv[0] = seg->data.line.points[2*i];
			datafv[1] = seg->data.line.points[2*i + 1];

			vgAppendPathData(path,1,&pathseg,datafv);
		}
		break;

	case PIL_UNKNOWN_SEG:
	default:
		break;
	}
}

// Go from parser-focused to OpenVG-focused structures
static gui_obj_t *convert_pil_object(pil_attr_t *root) {
	pil_seg_t *seg;
	gui_obj_t *obj;

	// Argument check (True Klingon Programmers ALWAYS WIN!)
	if(!root) return NULL;

	obj = calloc(1,sizeof *obj);

	while(root) {
		switch(root->type) {
		case PIL_EDGE: // Edge paint
			obj->edge = convert_pil_paint(root->data.paint);
			break;

		case PIL_FILL: // Fill paint
			obj->fill = convert_pil_paint(root->data.paint);
			break;

		case PIL_PATH: // Outline
			// Create a path if there isn't one
			if(!obj->path)
				obj->path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
					VG_PATH_DATATYPE_F,1.0,0.0,0,0,
					VG_PATH_CAPABILITY_ALL);

			for(seg = root->data.path; seg; seg = seg->next)
				append_pil_seg(obj->path,seg);
			break;

		case PIL_UNKNOWN_ATTR:
		default:
			break;
		}

		root = root->next;
	}

	return obj;
}

// Draw one thing
static void gui_draw_obj(gui_obj_t *obj) {
	// Argument check
	if(!obj) return;

	// How to draw
	vgSetPaint(obj->edge,VG_STROKE_PATH);
	vgSetPaint(obj->fill,VG_FILL_PATH);

	// What to draw
	vgDrawPath(obj->path,VG_FILL_PATH|VG_STROKE_PATH);
}

// Draw everything
void gui_draw() {
	vgClear(0,0,screenwidth,screenheight);

	gui_draw_obj(guiroot);

	display_update();
}

// Sets up the GUI from a PIL file and intializes OpenVG
void gui_init() {
	VGfloat rgba[4];

	// Import the GUI description
	pilin = fopen("angel.pil","r");
	if(!pilin) die("cannot open angel.pil");
	pilparse();

	guiroot = convert_pil_object(pilroot);

	rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0;
	vgSetfv(VG_CLEAR_COLOR,4,rgba);
}

void gui_stop() {
}

