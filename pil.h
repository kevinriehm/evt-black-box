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
			double r, g, b;
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
	PIL_EDGE,
	PIL_FILL,
	PIL_PATH
} pil_attr_type_t;

typedef struct pil_attr {
	pil_attr_type_t type;

	union {
		pil_seg_t *path;
		pil_paint_t *paint;
	} data;

	struct pil_attr *prev;
	struct pil_attr *next;
} pil_attr_t;


extern FILE *pilin; // Input file for the PIL parser


extern int pilparse();

