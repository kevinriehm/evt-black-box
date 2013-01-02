#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <VG/openvg.h>

#include "display.h"
#include "linked_list.h"
#include "main.h"
#include "pil.h"
#include "scheduler.h"


#define GUI_REFRESH_MS 1000


typedef struct gui_obj {
	char *name;

	VGPath path;

	VGfloat affine[9];

	int numchildren;
	struct gui_obj **children;

	double edgewidth;
	VGPaint edge;

	VGPaint fill;
} gui_obj_t;


int numclasses;
gui_obj_t *guiroot;
gui_obj_t **classes;


static gui_obj_t *convert_pil_object(pil_attr_t *, const gui_obj_t *);


static void add_class(pil_attr_t *attrs) {
	classes = realloc(classes,++numclasses*sizeof *classes);
	classes[numclasses - 1] = convert_pil_object(attrs,NULL);
}

// Basically just make a copy of the class' template object
static gui_obj_t *inst_class(char *class) {
	int i;
	gui_obj_t *obj;

	for(i = numclasses - 1; i >= 0; i--) {
		if(strcmp(class,classes[i]->name) == 0) {
			obj = malloc(sizeof *obj);
			memcpy(obj,classes[i],sizeof *obj);
			return obj;
		}
	}

	// No such class - return a generic GUI object
	return convert_pil_object(NULL,NULL);
}

static void add_child(gui_obj_t *obj, gui_obj_t *child) {
	obj->children = realloc(obj->children,
		++obj->numchildren*sizeof *obj->children);
	obj->children[obj->numchildren - 1] = child;
}

// m = m*n
// Also implicitly converts from PIL to OpenVG format
static void mult_matrix(VGfloat m[9], double n[3][3]) {
	int i, j;
	VGfloat r[9];

	for(i = 0; i < 3; i++) // Row of m
		for(j = 0; j < 3; j++) // Column of n
			r[3*j + i] = m[i]*n[0][j] + m[i + 3]*n[1][j]
				+ m[i + 6]*n[2][j];

	memcpy(m,r,9*sizeof *r);
}

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

static void apply_pil_attrs(gui_obj_t *obj, pil_attr_t *attr) {
	pil_seg_t *seg;
	gui_obj_t *child;
	pil_attr_t *oldattr;

	while(attr) {
		switch(attr->type) {
		case PIL_AFFINE: // Linear transformation
			mult_matrix(obj->affine,attr->data.affine);
			break;

		case PIL_CHILD: // Sub-object
			child = convert_pil_object(attr->data.child,obj);
			add_child(obj,child);
			break;

		case PIL_CLASS: // Object class
			add_class(attr->data.class);
			break;

		case PIL_EDGE: // Edge paint
			obj->edgewidth = attr->data.edge.width;
			obj->edge = convert_pil_paint(attr->data.edge.paint);
			break;

		case PIL_FILL: // Fill paint
			obj->fill = convert_pil_paint(attr->data.fill.paint);
			break;

		case PIL_INST: // Sub-object instantiated from a class
			child = inst_class(attr->data.inst.class);
			apply_pil_attrs(child,attr->data.inst.attrs);
			add_child(obj,child);
			break;

		case PIL_NAME: // Object name
			obj->name = attr->data.name;
			break;

		case PIL_PATH: // Outline
			// Create a path if there isn't one
			if(!obj->path)
				obj->path = vgCreatePath(
					VG_PATH_FORMAT_STANDARD,
					VG_PATH_DATATYPE_F,1.0,0.0,0,0,
					VG_PATH_CAPABILITY_ALL);

			for(seg = attr->data.path; seg; seg = seg->next)
				append_pil_seg(obj->path,seg);
			break;

		case PIL_UNKNOWN_ATTR:
		default:
			break;
		}

		oldattr = attr;
		attr = attr->next;
		free(oldattr);
	}
}

// Go from parser-focused to OpenVG-focused structures
static gui_obj_t *convert_pil_object(pil_attr_t *attrs,
	const gui_obj_t *parent) {
	gui_obj_t *obj;

	obj = calloc(1,sizeof *obj);

	// Set everything to reasonable defaults

	obj->affine[0] = obj->affine[4] = obj->affine[8] = 1; // Identity

	if(parent) { // Copy appearance from parent?
		obj->edgewidth = parent->edgewidth;
		obj->edge = parent->edge;
		obj->fill = parent->fill;
	} else { // Make something up
		obj->edgewidth = 0;
		obj->edge = vgCreatePaint();
		obj->fill = vgCreatePaint();
	}

	// Now, add in the specifics
	apply_pil_attrs(obj,attrs);

	return obj;
}

// Draw one thing
static void draw_obj(gui_obj_t *obj) {
	int i;
	VGfloat pathm[9];

	// Argument check
	if(!obj) return;

	// Save some state
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(pathm);


	// How to draw

	vgSetf(VG_STROKE_LINE_WIDTH,obj->edgewidth);
	vgSetPaint(obj->edge,VG_STROKE_PATH);

	vgSetPaint(obj->fill,VG_FILL_PATH);

	vgMultMatrix(obj->affine); // Linear transformation

	// What to draw
	vgDrawPath(obj->path,VG_FILL_PATH|VG_STROKE_PATH);


	// What else to draw
	for(i = 0; i < obj->numchildren; i++)
		draw_obj(obj->children[i]);


	// Restore some state
	vgLoadMatrix(pathm);
}

// Draw all things
void gui_draw() {
	vgClear(0,0,screenwidth,screenheight);

	// Reset transformations
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();

	draw_obj(guiroot);

	display_update();
}

// Sets up the GUI from a PIL file and intializes OpenVG
void gui_init() {
	VGfloat rgba[4];

	// Import the GUI description

	pilin = fopen("angel.pil","r");
	if(!pilin) die("cannot open angel.pil");
	pilparse();

	numclasses = 0;
	classes = NULL;

	guiroot = convert_pil_object(pilroot,NULL);

	// Set up OpenVG stuff

	rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0;
	vgSetfv(VG_CLEAR_COLOR,4,rgba);
}

void gui_stop() {
}

