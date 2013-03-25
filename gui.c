#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <VG/openvg.h>

#include "angel.h"
#include "display.h"
#include "libs.h"
#include "linked_list.h"
#include "pil.h"
#include "scheduler.h"


#define GUI_REFRESH_MS 1000


// Visual state for one object
typedef struct {
	char *name;

	struct gui_obj *owner; // To facilitate copy-on-write

	VGPath path;

	VGfloat affine[9];

	double edgewidth;
	VGPaint edge;

	VGPaint fill;

	// State changes
	struct {
		int press;
	} on;
} gui_state_t;

typedef struct gui_obj {
	char *name;

	int curstate;
	int numstates;
	gui_state_t **states;

	int numchildren;
	struct gui_obj **children;
} gui_obj_t;


static int numclasses;
static gui_obj_t **classes;

static gui_obj_t *guiroot;

static int realwidth, realheight;
static int logicalwidth, logicalheight;


static int add_state(gui_obj_t *, gui_state_t *);
static gui_obj_t *convert_pil_object(pil_attr_t *, const gui_obj_t *);


static gui_state_t *new_state() {
	return calloc(1,sizeof(gui_state_t));
}

static gui_obj_t *new_obj() {
	return calloc(1,sizeof(gui_obj_t));
}

// Returns dest
static gui_state_t *copy_state(gui_state_t *dest, gui_state_t *src) {
	gui_obj_t *owner;

	owner = dest->owner; // Preserve ownership of dest
	*dest = *src; // The basics
	dest->owner = owner;

	// Give dest its own buffers

	if(src->name) {
		dest->name = malloc(strlen(src->name)*sizeof *src->name);
		strcpy(dest->name,src->name);
	}

	return dest;
}

// Returns dest
static gui_obj_t *copy_obj(gui_obj_t *dest, gui_obj_t *src) {
	size_t size;

	*dest = *src; // The basics

	// Give dest its own buffers

	if(src->name) {
		dest->name = malloc(strlen(src->name)*sizeof *src->name);
		strcpy(dest->name,src->name);
	}

	size = src->numstates*sizeof *src->states;
	dest->states = malloc(size);
	memcpy(dest->states,src->states,size);

	size = src->numchildren*sizeof *src->children;
	dest->children = malloc(size);
	memcpy(dest->children,src->children,size);

	return dest;
}

static void add_class(gui_obj_t *obj) {
	classes = realloc(classes,++numclasses*sizeof *classes);
	classes[numclasses - 1] = obj;
}

// Basically just make a copy of the class' template object
static gui_obj_t *inst_class(char *class) {
	int i;

	for(i = numclasses - 1; i >= 0; i--)
		if(strcmp(class,classes[i]->name) == 0)
			return copy_obj(new_obj(),classes[i]); // Found it!

	// No such class - return a generic GUI object
	return convert_pil_object(NULL,NULL);
}

static void add_child(gui_obj_t *obj, gui_obj_t *child) {
	obj->children = realloc(obj->children,
		(++obj->numchildren + 1)*sizeof *obj->children);
	obj->children[obj->numchildren - 1] = child;
}

// Returns the state's index
static int add_state(gui_obj_t *obj, gui_state_t *state) {
	if(!state->owner)
		state->owner = obj;

	obj->states = realloc(obj->states,
		(++obj->numstates + 1)*sizeof *obj->states);
	obj->states[obj->numstates - 1] = state;

	return obj->numstates - 1;
}

// Returns the index of the entry matching targetname in i, or -1 if none
// find_name(int i, struct **list, char *name)
#define find_name(i, list, targetname) { \
	for((i) = 0; (list) && (list)[i] \
		&& strcmp((list)[i]->name,(targetname)) != 0; (i)++); \
	if(!(list) || !(list)[i]) (i) = -1; \
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

// Coordinates PIL structure conversion
static void apply_pil_attrs(gui_obj_t *obj, int statei,
		pil_attr_t *attr) {
	int *si, i;
	pil_seg_t *seg;
	gui_obj_t *child;
	gui_state_t *state;
	pil_attr_t *oldattr;

	state = obj->states[statei];

	if(state->owner != obj) // Copy on write
		state = obj->states[statei] = copy_state(new_state(),state);

	while(attr) {
		switch(attr->type) {
		case PIL_AFFINE: // Linear transformation
			if(!state) break;
			mult_matrix(state->affine,attr->data.affine);
			break;

		case PIL_CHILD: // Sub-object
			if(!obj) break;

			find_name(i,obj->children,attr->data.child->data.name);

			if(i < 0) {
				child = convert_pil_object(attr->data.child,
					obj);
				add_child(obj,child);
			} else apply_pil_attrs(obj->children[i],0,
				attr->data.child);
			break;

		case PIL_CLASS: // Object class
			add_class(convert_pil_object(attr->data.class,obj));
			break;

		case PIL_EDGE: // Edge paint
			if(!state) break;
			state->edgewidth = attr->data.edge.width;
			state->edge = convert_pil_paint(attr->data.edge.paint);
			break;

		case PIL_EVENT:
			if(!obj || !state) break;

			switch(attr->data.event.type) {
			case EVENT_PRESS: si = &state->on.press; break;
			default: si = &i; break;
			}

			find_name(*si,obj->states,attr->data.event.nextstate);
			break;

		case PIL_FILL: // Fill paint
			if(!state) break;
			state->fill = convert_pil_paint(attr->data.fill.paint);
			break;

		case PIL_INST: // Sub-object instantiated from a class
			if(!obj) break;

			find_name(i,obj->children,
				attr->data.inst.attrs->data.name);

			if(i < 0) {
				child = inst_class(attr->data.inst.class);
				apply_pil_attrs(child,0,attr->data.inst.attrs);
				add_child(obj,child);
			} else apply_pil_attrs(obj->children[i],0,
				attr->data.inst.attrs);
			break;

		case PIL_NAME: // Object name
			if(!state) break;
			if(state->name && attr->data.name
				&& strcmp(state->name,attr->data.name) == 0)
				free(attr->data.name);
			else {
				state->name = attr->data.name;

				if(statei == 0 && obj) obj->name = state->name;
			}
			break;

		case PIL_PATH: // Outline
			if(!state) break;

			// Create a path if there isn't one
			if(!state->path)
				state->path = vgCreatePath(
					VG_PATH_FORMAT_STANDARD,
					VG_PATH_DATATYPE_F,1.0,0.0,0,0,
					VG_PATH_CAPABILITY_ALL);

			for(seg = attr->data.path; seg; seg = seg->next)
				append_pil_seg(state->path,seg);
			break;

		case PIL_STATE: // Animation state
			if(!obj) break;

			find_name(i,obj->states,attr->data.state->data.name);

			if(i < 0)
				apply_pil_attrs(obj,add_state(
						obj,
						copy_state(new_state(),state)
					),
					attr->data.state);
			else apply_pil_attrs(obj,i,attr->data.state);
			break;

		case PIL_WINDOW: // Logical window
			logicalwidth = attr->data.window.width;
			logicalheight = attr->data.window.height;

			if(logicalwidth < 1) logicalwidth = 1;
			if(logicalheight < 1) logicalheight = 1;
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
	gui_state_t *state;

	obj = new_obj();

	// Set everything to reasonable defaults

	if(parent) { // Copy appearance from parent?
		add_state(obj,parent->states[0]);
	} else { // Make something up
		state = new_state();
		add_state(obj,state);

		state->edgewidth = 0;
		state->edge = vgCreatePaint();
		state->fill = vgCreatePaint();

		// Identity transformation matrix
		state->affine[0] = state->affine[4] = state->affine[8] = 1;
	}

	// Now, add in the specifics
	apply_pil_attrs(obj,0,attrs);

	return obj;
}

// Draw one thing
static void draw_obj(gui_obj_t *obj) {
	int i;
	VGfloat pathm[9];
	gui_state_t *state;

	// Argument check
	if(!obj) return;

	state = obj->states[obj->curstate];

	// Save some state
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(pathm);


	// How to draw
	vgSetf(VG_STROKE_LINE_WIDTH,state->edgewidth);
	vgSetPaint(state->edge,VG_STROKE_PATH);

	vgSetPaint(state->fill,VG_FILL_PATH);

	vgMultMatrix(state->affine); // Linear transformation

	// What to draw
	if(state->path)
		vgDrawPath(state->path,VG_FILL_PATH|VG_STROKE_PATH);


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
	vgScale((float) realwidth/logicalwidth,
		(float) realheight/logicalheight);

	draw_obj(guiroot);

	display_update();
}

// Update realwidth and realheight
void gui_handle_resize(int w, int h) {
	realwidth = w;
	realheight = h;
}

// Sets up the GUI from a PIL file and intializes OpenVG
void gui_init() {
	VGfloat rgba[4];

	realwidth = logicalwidth = 640;   // Completely
	realheight = logicalheight = 480; // Arbitrary

	display_init(realwidth,realheight);

	// Import the GUI description

	pilin = fopen("angel.pil","r");
	if(!pilin) die("cannot open angel.pil");
	pilparse();

	numclasses = 0;
	classes = NULL;

	guiroot = convert_pil_object(pilroot,NULL);

	// Set up OpenVG stuff

	rgba[0] = rgba[1] = rgba[2] = 0, rgba[3] = 1;
	vgSetfv(VG_CLEAR_COLOR,4,rgba);
}

void gui_stop() {
	display_stop();
}

