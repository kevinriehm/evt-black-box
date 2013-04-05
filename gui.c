#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <EGL/egl.h>
#include <png.h>
#include <VG/openvg.h>

#include "angel.h"
#include "display.h"
#include "event.h"
#include "font.h"
#include "gui.h"
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

	struct { // State changes
		int state;
		int trigger;

		float timeout;
		clock_t start;
	} on[GUI_NUM_EVENTS];

	struct { // Bounding box
		VGfloat x, y;
		VGfloat w, h;
	} box;

	struct {
		enum { GUI_NONE, GUI_BIGIMAGE, GUI_PRINTF, GUI_ROTATE } type;
		char *text;
		double m[2], b[2];
	} value;
} gui_state_t;

typedef struct gui_obj {
	char *name;

	VGfloat affine[9]; // Applies to all states

	double value[3];

	int curstate;
	int numstates;
	gui_state_t **states;

	int numchildren;
	struct gui_obj **children;
} gui_obj_t;


static int mainfont;

static int numclasses;
static gui_obj_t **classes;

static gui_obj_t *guiroot;

static int realwidth, realheight;
static int logicalwidth, logicalheight;

static int numtriggers;
static struct {
	char *name;
	void (*f)();
} *triggers;


static int add_state(gui_obj_t *, gui_state_t *);
static gui_obj_t *convert_pil_object(pil_attr_t *, const gui_obj_t *);
static int add_trigger(char *name, void (*func)());


static gui_state_t *new_state() {
	int i;
	gui_state_t *state;

	state = calloc(1,sizeof(gui_state_t));

	state->affine[0] = state->affine[4] = state->affine[8] = 1;

	for(i = 0; i < GUI_NUM_EVENTS; i++) {
		state->on[i].state = -1;
		state->on[i].trigger = 1;
	}

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

// Returns a pointer to an object named name
gui_obj_t *gui_find_obj(char *name, gui_obj_t *root) {
	int i;
	gui_obj_t *obj;

	if(!root) root = guiroot;

	for(i = 0; i < root->numchildren; i++)
		if(strcmp(name,root->children[i]->name) == 0)
			return root->children[i];
		else if(obj = gui_find_obj(name,root->children[i]))
			return obj;

	return NULL;
}

void gui_set_value(gui_obj_t *obj, ...) {
	static const int numvalues[] = {
		[GUI_NONE] = 0,
		[GUI_BIGIMAGE] = 3,
		[GUI_PRINTF] = 1,
		[GUI_ROTATE] = 1
	};

	int i;
	va_list ap;

	va_start(ap,obj);

	for(i = 0; i < numvalues[obj->states[obj->curstate]->value.type];
			i++)
		obj->value[i] = va_arg(ap,double);

	va_end(ap);

	event_redraw();
}

static VGImage get_image(char * dir, int x, int y, int z) {
	static const int maxlevels = 16;

	static VGImage ***images;

	FILE *imgfp;
	int i, bd, ct;
	VGImage image;
	png_bytep *rows;
	png_structp pngp;
	png_uint_32 w, h;
	char buf[FILENAME_MAX];
	png_infop endinfop, infop;

	if(z < 0 || z >= maxlevels || x < 0 || x >= 1 << z || y < 0
			|| y >= 1 << z)
		return VG_INVALID_HANDLE; // Nope

	if(images && images[z] && images[z][x] && images[z][x][y])
		return images[z][x][y]; // Cached and ready to go!

	// Populate the cache structure
	if(!images) images = calloc(maxlevels,sizeof *images);
	if(!images[z]) images[z] = calloc(1 << z,sizeof **images);
	if(!images[z][x]) images[z][x] = calloc(1 << z,sizeof ***images);

	// Load the image from a file

	sprintf(buf,"%s/%i/%i/%i.png",dir,z,x,y);
	imgfp = fopen(buf,"rb");
	if(!imgfp) {
		printf("cannot open '%s'\n",buf);
		return VG_INVALID_HANDLE;
	}

	pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	infop = png_create_info_struct(pngp);
	endinfop = png_create_info_struct(pngp);

	if(setjmp(png_jmpbuf(pngp))) {
		printf("libpng error while reading file '%s'\n",buf);
		png_destroy_read_struct(&pngp,&infop,&endinfop);
		fclose(imgfp);
		return VG_INVALID_HANDLE;
	}

	png_init_io(pngp,imgfp);
	png_read_png(pngp,infop,PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING,
		NULL);
	rows = png_get_rows(pngp,infop);
	png_get_IHDR(pngp,infop,&w,&h,&bd,&ct,NULL,NULL,NULL);

	image = vgCreateImage(VG_sRGBA_8888,w,h,VG_IMAGE_QUALITY_FASTER);
	for(i = 0; i < h; i++)
		vgImageSubData(image,rows[i],0,VG_sRGBA_8888,0,h - i - 1,w,1);

	png_read_end(pngp,endinfop);
	png_destroy_read_struct(&pngp,&infop,&endinfop);

	fclose(imgfp);

	images[z][x][y] = image;

	return image;
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
	VGfloat datafv[4];

	empty = vgGetParameteri(path,VG_PATH_NUM_COORDS) == 0;

	switch(seg->type) {
	case PIL_CLOSE:
		pathseg = VG_CLOSE_PATH;
		vgAppendPathData(path,1,&pathseg,NULL);
		break;

	case PIL_LINE:
		for(i = 0; i < seg->data.line.numpoints; i++) {
			pathseg = empty ? VG_MOVE_TO : VG_LINE_TO;
			datafv[0] = seg->data.line.points[2*i + 0];
			datafv[1] = seg->data.line.points[2*i + 1];

			vgAppendPathData(path,1,&pathseg,datafv);

			empty = 0;
		}
		break;

	case PIL_MOVE_TO:
		pathseg = VG_MOVE_TO;
		datafv[0] = seg->data.moveto.x;
		datafv[1] = seg->data.moveto.y;

		vgAppendPathData(path,1,&pathseg,datafv);
		break;

	case PIL_QUAD_BEZIER:
		for(i = 0; i < seg->data.quadbezier.numpoints/2; i++) {
			pathseg = empty ? VG_MOVE_TO : VG_QUAD_TO;
			datafv[0] = seg->data.quadbezier.points[4*i + 0];
			datafv[1] = seg->data.quadbezier.points[4*i + 1];
			datafv[2] = seg->data.quadbezier.points[4*i + 2];
			datafv[3] = seg->data.quadbezier.points[4*i + 3];

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
	pil_seg_t *seg;
	gui_obj_t *child;
	gui_state_t *state;
	pil_attr_t *oldattr;
	int *si, *ti, i, junki;

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
				si = &state->on[GUI_PRESS].state;
				ti = &state->on[GUI_PRESS].trigger;
				break;

			case EVENT_RELEASE:
				si = &state->on[GUI_RELEASE].state;
				ti = &state->on[GUI_RELEASE].trigger;
				break;

			case EVENT_TIMEOUT:
				si = &state->on[GUI_TIMEOUT].state;
				ti = &state->on[GUI_TIMEOUT].state;

				state->on[GUI_TIMEOUT].timeout 
					= attr->data.event.timeout;
				state->on[GUI_TIMEOUT].start = clock();
				break;

			default: si = ti = &junki; break;
			}

			find_name(*si,obj->states,attr->data.event.nextstate);

			for(i = 0; i < numtriggers; i++)
				if(strcmp(triggers[i].name,
					attr->data.event.trigger) == 0)
					break;

			if(i < numtriggers) *ti = i;
			else *ti = add_trigger(attr->data.event.trigger,NULL);
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

		case PIL_VALUE:
			state->value.type = attr->data.value->type;
			state->value.text = attr->data.value->text;
			state->value.m[0] = attr->data.value->scale[0];
			state->value.b[0] = attr->data.value->offset[0];
			state->value.m[1] = attr->data.value->scale[1];
			state->value.b[1] = attr->data.value->offset[1];
			free(attr->data.value);
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

static void pull_trigger(gui_obj_t *obj, enum gui_event event) {
	int trigger;

	trigger = obj->states[obj->curstate]->on[event].trigger;
	if(trigger >= 0 && trigger < numtriggers && triggers[trigger].f)
		triggers[trigger].f();

	if(obj->states[obj->curstate]->on[event].state >= 0)
		obj->curstate = obj->states[obj->curstate]->on[event].state;

	obj->states[obj->curstate]->on[GUI_TIMEOUT].start = clock();

	event_redraw();
}

// Draw one thing
static void draw_obj(gui_obj_t *obj) {
	// 64K should be enough for anyone...
	static char buf[64*1024];

	int i, z;
	double x, y;
	VGImage image;
	VGfloat pathm[9];
	gui_state_t *state;

	// Argument check
	if(!obj) return;

	state = obj->states[obj->curstate];

	// Handle timeouts here because it's convenient
	if(state->on[GUI_TIMEOUT].state >= 0)
		event_set_max_wait(state->on[GUI_TIMEOUT].timeout);

	if(state->on[GUI_TIMEOUT].timeout*CLOCKS_PER_SEC
		< clock() - state->on[GUI_TIMEOUT].start)
		pull_trigger(obj,GUI_TIMEOUT);

	// Save some state
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgGetMatrix(pathm);


	// How to draw
	vgSetf(VG_STROKE_LINE_WIDTH,state->edgewidth);
	vgSetPaint(state->edge,VG_STROKE_PATH);

	vgSetPaint(state->fill,VG_FILL_PATH);

	// Transformations
	vgMultMatrix(obj->affine); // Object transformation
	vgMultMatrix(state->affine); // State transformation

	switch(state->value.type) {
	case GUI_BIGIMAGE:
		vgSeti(VG_MATRIX_MODE,VG_MATRIX_IMAGE_USER_TO_SURFACE);
		vgLoadIdentity();

		x = state->value.m[0]*obj->value[0] + state->value.b[0];
		y = state->value.m[1]*obj->value[1] + state->value.b[1];
		z = obj->value[2];

		x *= 1 << z;
		y *= 1 << z;

		vgTranslate(state->box.x + state->box.w/2,
			state->box.y + state->box.h/2);

		image = get_image(state->value.text,x,y,z);

		vgTranslate(-(x - (int64_t) x)
			*vgGetParameteri(image,VG_IMAGE_WIDTH),
			-(y - (int64_t) y)
			*vgGetParameteri(image,VG_IMAGE_HEIGHT));

		vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
		break;

	case GUI_PRINTF:
		sprintf(buf,state->value.text,obj->value);
		font_print(mainfont,0,0,buf);
		break;

	case GUI_ROTATE:
		vgRotate(state->value.m[0]*obj->value[0] + state->value.b[0]);
		break;

	case GUI_NONE:
	default: break;
	}

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
clock_t c = clock();
	vgClear(0,0,realwidth,realheight);

	// Reset transformations
	vgSeti(VG_MATRIX_MODE,VG_MATRIX_PATH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale((float) realwidth/logicalwidth,
		(float) realheight/logicalheight);

	draw_obj(guiroot);

	display_update();
printf("%.2f seconds\n",(float) (clock() - c)/CLOCKS_PER_SEC);
}

static int add_trigger(char *name, void (*func)()) {
	triggers = realloc(triggers,++numtriggers*sizeof *triggers);
	triggers[numtriggers - 1].name = name;
	triggers[numtriggers - 1].f = func;

	return numtriggers - 1;
}

// Set the function for a trigger
void gui_bind(char *trigger, void (*func)()) {
	int i;

	for(i = 0; i < numtriggers; i++)
		if(strcmp(trigger,triggers[i].name) == 0)
			break;

	if(i < numtriggers) triggers[i].f = func; // The slot already exists
	else add_trigger(trigger,func); // Or not...
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
	for(i = obj->numchildren - 1; i >= 0; i--)
		if(o = obj_coords(obj->children[i],event,x,y))
			return o;

	// Now check obj
	if((state->on[event].state >= 0 || state->on[event].trigger >= 0)
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

	if(obj) pull_trigger(obj,event);
}

// Update realwidth and realheight
void gui_handle_resize(int w, int h) {
	realwidth = w;
	realheight = h;
}

// Sets up the GUI from a PIL file and intializes OpenVG
void gui_init() {
	VGfloat rgba[4];

	// Prepare the display

	logicalwidth = 800;
	logicalheight = 480;

	realwidth = 800;
	realheight = 600;

	display_init(realwidth,realheight);

	// Get fonts ready

	mainfont = font_load("FreeSans.ttf");

	// Import the GUI description

	pilin = fopen("angel.pil","r");
	if(!pilin) die("cannot open angel.pil");
	pilparse();

	numclasses = 0;
	classes = NULL;

	numtriggers = 0;
	triggers = NULL;

	guiroot = convert_pil_object(pilroot,NULL);

	// Set up OpenVG stuff

	rgba[0] = rgba[1] = rgba[2] = 0, rgba[3] = 1;
	vgSetfv(VG_CLEAR_COLOR,4,rgba);
}

void gui_stop() {
	display_stop();
}

