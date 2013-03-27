#include <stdio.h>


// Paint: what something should look like
typedef enum {
	PIL_UNKNOWN_PAINT = 0,
	PIL_COLOR
} pil_paint_type_t;

typedef struct {
	pil_paint_type_t type;

	union {
		struct {
			double r, g, b, a;
		} color;
	} data;
} pil_paint_t;


// Segment: building block for paths
typedef enum {
	PIL_UNKNOWN_SEG = 0,
	PIL_LINE
} pil_seg_type_t;

typedef struct pil_seg {
	pil_seg_type_t type;

	union {
		struct {
			int numpoints;
			double *points; // Point = pair of coordinates
		} line;
	} data;

	struct pil_seg *prev;
	struct pil_seg *next;
} pil_seg_t;


// Attributes: everything is an attribute of something
typedef enum {
	PIL_UNKNOWN_ATTR = 0,
	PIL_AFFINE,
	PIL_CHILD,
	PIL_CLASS,
	PIL_EDGE,
	PIL_EVENT,
	PIL_FILL,
	PIL_INST,
	PIL_NAME,
	PIL_PATH,
	PIL_STATE,
	PIL_WINDOW
} pil_attr_type_t;

typedef enum {
	EVENT_PRESS,
	EVENT_RELEASE
} pil_event_type_t;

typedef struct pil_attr {
	pil_attr_type_t type;

	union {
		double affine[3][3];

		struct pil_attr *child;

		struct pil_attr *class;

		struct {
			double width;
			pil_paint_t *paint;
		} edge;

		struct {
			pil_event_type_t type;
			char *nextstate;
		} event;

		struct {
			pil_paint_t *paint;
		} fill;

		struct {
			char *class;
			struct pil_attr *attrs;
		} inst;

		char *name;

		pil_seg_t *path;

		struct pil_attr *state;

		struct {
			int width;
			int height;
		} window;
	} data;

	struct pil_attr *prev;
	struct pil_attr *next;
} pil_attr_t;


extern FILE *pilin; // Input file for the PIL parser
extern pil_attr_t *pilroot; // Output structure
extern int pilline; // Line number during lexing


extern int pillex();
extern int pilparse();

