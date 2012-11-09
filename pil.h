// Attributes: everything is an attribute of something
typedef enum {
	PIL_UNKNOWN = 0,
	PIL_EDGE,
	PIL_FILL,
	PIL_PATH
} pil_attr_type_t;

typedef struct pil_attr {
	struct *pil_attr prev;
	struct *pil_attr next;
} pil_attr_t;


// Paint: what something should look like
typedef enum {
	PIL_UNKNOWN = 0,
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

