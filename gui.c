#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <VG/openvg.h>

#include "angel.h"
#include "display.h"
#include "event.h"
#include "gui.h"
#include "libs.h"
#include "linked_list.h"
#include "pil.h"
#include "scheduler.h"


#define GUI_REFRESH_MS 1000


// Visual state for one object
typedef struct gui_state {
	char *name;

	struct gui_state **home; // To facilitate copy-on-write

	int ownpath;
	VGPath path;

	VGfloat affine[9];

	double edgewidth;
	VGPaint edge;

	VGPaint fill;

	int on[GUI_NUM_EVENTS]; // State changes

	struct { // Bounding box
		VGfloat x, y;
		VGfloat w, h;
	} box;
} gui_state_t;

typedef struct gui_obj {
	char *name;

	VGfloat affine[9]; // Applies to all states

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
	int i;
	gui_state_t *state;

	state = calloc(1,sizeof(gui_state_t));

	state->affine[0] = state->affine[4] = state->affine[8] = 1;

	for(i = 0; i < GUI_NUM_EVENTS; i++)
		state->on[i] = -1;

	return state;
}

static gui_obj_t *new_obj() {
	gui_obj_t *obj = calloc(1,sizeof(gui_obj_t));
	obj->affine[0] = obj->affine[4] = obj->affine[8] = 1;
	return obj;
}

static VGPaint dup_paint(VGPaint src) {
	VGPaint paint = vgCreatePaint();
	vgSetColor(paint,vgGetColor(src));
	return paint;
}

// Returns dest
static gui_state_t *copy_state(gui_state_t *dest, gui_state_t *src) {
	*dest = *src; // The basics
	dest->home = NULL; // No schizophrenia allowed

	dest->ownpath = 0; // No longer owner of path

	// Give dest its own memory

	if(src->name) {
		dest->name = malloc(strlen(src->name)*sizeof *src->name);
		strcpy(dest->name,src->name);
	}

	dest->edge = dup_paint(dest->edge);
	dest->fill = dup_paint(dest->fill);

	return dest;
}

// Returns dest
static gui_obj_t *copy_obj(gui_obj_t *dest, gui_obj_t *src) {
	size_t size;

	*dest = *src; // The basics

	// Give dest its own memory

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
	obj->children[obj->numchildren] = NULL;
}

// Returns the state's index
static int add_state(gui_obj_t *obj, gui_state_t *state) {
	obj->states = realloc(obj->states,
		(++obj->numstates + 1)*sizeof *obj->states);
	obj->states[obj->numstates - 1] = state;
	obj->states[obj->numstates] = NULL;

	if(!state->home) // Home is where the pointer is
		state->home = obj->states + obj->numstates - 1;

	if(obj->numstates == 1)
		state->name = "default";

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

static void inverted_affine(float *x, float *y, VGfloat a[static 9]) {
	float det, tx, ty;

	det = a[0]*a[4] - a[1]*a[3];
	tx = ( *x*a[4] - *y*a[3] + a[7]*a[3] - a[4]*a[6])/det;
	ty = (-*x*a[1] + *y*a[0] - a[7]*a[0] + a[1]*a[6])/det;

	*x = tx;
	*y = ty;
}

static void append_pil_seg(VGPath path, pil_seg_t *seg) {
	int empty, i;
	VGubyte pathseg;
	VGfloat datafv[2];

	empty = vgGetParameteri(path,VG_PATH_NUM_COORDS) == 0;

	switch(seg->type) {
	case PIL_LINE:
		for(i = 0; i < seg->data.line.numpoints; i++) {
			pathseg = empty ? VG_MOVE_TO : VG_LINE_TO;
			datafv[0] = seg->data.line.points[2*i];
			datafv[1] = seg->data.line.points[2*i + 1];

			vgAppendPathData(path,1,&pathseg,datafv);

			empty = 0;
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

	if(state->home != obj->states + statei) // Copy on write
		state = obj->states[statei] = copy_state(new_state(),state);

	while(attr) {
		switch(attr->type) {
		case PIL_AFFINE: // Linear transformation
			if(!state) break;
			if(obj && statei == 0)
				mult_matrix(obj->affine,attr->data.affine);
			else mult_matrix(state->affine,attr->data.affine);
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
			if(state->edge) vgDestroyPaint(state->edge);
			state->edgewidth = attr->data.edge.width;
			state->edge = convert_pil_paint(attr->data.edge.paint);
			break;

		case PIL_EVENT:
			if(!obj || !state) break;

			switch(attr->data.event.type) {
			case EVENT_PRESS:
				si = state->on + GUI_PRESS;
				break;

			case EVENT_RELEASE:
				si = state->on + GUI_RELEASE;
				break;

			default: si = &i; break;
			}

			find_name(*si,obj->states,attr->data.event.nextstate);
			break;

		case PIL_FILL: // Fill paint
			if(!state) break;
			if(state->fill) vgDestroyPaint(state->fill);
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
			else if(statei == 0) obj->name = attr->data.name;
			else state->name = attr->data.name;
			break;

		case PIL_PATH: // Outline
			if(!state) break;

			// Create a path if there isn't one
			// or obj doesn't own it
			if(!state->path || !state->ownpath) {
				state->path = vgCreatePath(
					VG_PATH_FORMAT_STANDARD,
					VG_PATH_DATATYPE_F,1.0,0.0,0,0,
					VG_PATH_CAPABILITY_ALL);
				state->ownpath = 1;
			}

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

		// Store a bounding box now for convenience
		vgPathBounds(state->path,&state->box.x,&state->box.y,
			&state->box.w,&state->box.h);

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
		state = new_state();
		copy_state(state,parent->states[0]);
		add_state(obj,state);
	} else { // Make something up
		state = new_state();
		add_state(obj,state);

		state->edgewidth = 0;
		state->edge = vgCreatePaint();
		state->fill = vgCreatePaint();
	}

	// Fresh new transformation matrix
	state->affine[1] = state->affine[2] = state->affine[3]
		= state->affine[5] = state->affine[6] = state->affine[7] = 0;
	state->affine[0] = state->affine[4] = state->affine[8] = 1;

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

	vgMultMatrix(obj->affine); // Object transformation
	vgMultMatrix(state->affine); // State transformation

	// What to draw
	if(state->path)
		vgDrawPath(state->path,VG_FILL_PATH | VG_STROKE_PATH);

	// What else to draw
	for(i = 0; i < obj->numchildren; i++)
		draw_obj(obj->children[i]);


	// Restore some state
	vgLoadMatrix(pathm);
}

// Draw all things
void gui_draw() {
	vgClear(0,0,realwidth,realheight);

	// Reset transformations
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float) realwidth/logicalwidth,
		(float) realheight/logicalheight);

	draw_obj(guiroot);

	display_update();
}

// Pin xy coordinates to an object
// x and y are logical coordinates
static gui_obj_t *obj_coords(gui_obj_t *obj, enum gui_event event, float x,
		float y) {
	int i;
	gui_obj_t *o;
	gui_state_t *state;

	state = obj->states[obj->curstate]; // Shortcut

	// Inverse transform
	inverted_affine(&x,&y,obj->affine);
	inverted_affine(&x,&y,state->affine);

	// Children are on top; check them first
	for(i = 0; i < obj->numchildren; i++)
		if(o = obj_coords(obj->children[i],event,x,y))
			return o;

	// Now check obj
	if(state->on[event] >= 0
		&& state->box.x <= x && x <= state->box.x + state->box.w
		&& state->box.y <= y && y <= state->box.y + state->box.h)
		return obj;

	// Nothing, nada, nil
	return NULL;
}

// Trigger the state change
// x and y are real coordinates
void gui_handle_pointer(enum gui_event event, int x, int y) {
	gui_obj_t *obj;

	// Check everything
	obj = obj_coords(guiroot,event,(float) x*logicalwidth/realwidth,
		(1 - (float) y/realheight)*logicalheight);

	if(!obj) return; // Nothing was affected

	obj->curstate = obj->states[obj->curstate]->on[event];

	event_redraw();
}

// Update realwidth and realheight
void gui_handle_resize(int w, int h) {
	realwidth = w;
	realheight = h;
}

// Sets up the GUI from a PIL file and intializes OpenVG
void gui_init() {
	VGfloat rgba[4];

	realwidth = logicalwidth = 800;   // Completely
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

