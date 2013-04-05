%{
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "linked_list.h"
#include "pil.h"
%}

%{
#define PI 3.14159265358979323846264
%}

%union {
	char *string;
	double number;
	pil_seg_t *segment;
	pil_paint_t *paint;
	pil_value_t *value;
	pil_attr_t *attribute;
	pil_event_type_t event;

	struct {
		int count;
		int buflen;
		double *buf;
	} numbers;
}

/* Keywords */
%token CLASS STATE
%token AFTER EDGE FILL ON PATH ROTATE SCALE SHEAR TRANSLATE VALUE
%token  WINDOW X Y
%token PRESS RELEASE SECONDS
%token PX
%token RGB RGBA
%token CLOSE LINE QBEZIER TO
%token BIGIMAGE PRINTF FROM AT

/* Etc. */
%token <string>   IDENTIFIER STRING
%token <number>   NUMBER

/* Nonterminal types */
%type <number>    timespec length
%type <value>     valuespec
%type <segment>   pathspec segment
%type <paint>     paintspec
%type <attribute> attribute attributes objectbody
%type <event>     event
%type <numbers>   points

%{
void yyerror(const char *msg);


pil_paint_t *new_paint(pil_paint_type_t type, ...);
pil_seg_t *new_seg(pil_seg_type_t type);
pil_attr_t *new_attr(pil_attr_type_t type, ...);


pil_attr_t *pilroot;
%}

%%

root: attributes { LL_FIND_HEAD(pilroot,$1); };

attributes: { $$ = NULL; }
	| attributes attribute {
		$$ = $2;
		LL_APPEND($1,$$);
	};

attribute:
	  AFTER timespec ':' IDENTIFIER IDENTIFIER '(' ')' {
		$$ = new_attr(PIL_EVENT,EVENT_TIMEOUT,$4,$5,$2);
	}
	| EDGE ':' length paintspec {
		$$ = new_attr(PIL_EDGE,$3,$4);
	}
	| FILL ':' paintspec {
		$$ = new_attr(PIL_FILL,$3);
	}
	| ON event ':' IDENTIFIER IDENTIFIER '(' ')' {
		$$ = new_attr(PIL_EVENT,$2,$4,$5);
	}
	| PATH ':' pathspec {
		LL_FIND_HEAD($3,$3);
		$$ = new_attr(PIL_PATH,$3);
	}
	| ROTATE ':' NUMBER {
		double rad = $3*PI/180;
		$$ = new_attr(PIL_AFFINE,
			cos(rad),-sin(rad),0.0,
			sin(rad), cos(rad),0.0,
			     0.0,      0.0,1.0
		);
	}
	| SCALE ':' NUMBER {
		$$ = new_attr(PIL_AFFINE,
			 $3,0.0,0.0,
			0.0, $3,0.0,
			0.0,0.0,1.0
		);
	}
	| SCALE ':' NUMBER NUMBER {
		$$ = new_attr(PIL_AFFINE,
			 $3,0.0,0.0,
			0.0, $4,0.0,
			0.0,0.0,1.0
		);
	}
	| SHEAR ':' NUMBER NUMBER {
		$$ = new_attr(PIL_AFFINE,
			1.0, $3,0.0,
			 $4,1.0,0.0,
			0.0,0.0,1.0
		);
	}
	| TRANSLATE ':' NUMBER NUMBER {
		$$ = new_attr(PIL_AFFINE,
			1.0,0.0, $3,
			0.0,1.0, $4,
			0.0,0.0,1.0
		);
	}
	| VALUE ':' valuespec {
		$$ = new_attr(PIL_VALUE,$3);
	}
	| WINDOW ':' NUMBER NUMBER {
		$$ = new_attr(PIL_WINDOW,$3,$4);
	}
	| X ':' NUMBER { // Equivalent to 'translate: NUMBER 0'
		$$ = new_attr(PIL_AFFINE,
			1.0,0.0, $3,
			0.0,1.0,0.0,
			0.0,0.0,1.0
		);
	}
	| Y ':' NUMBER { // Equivalent to 'translate: 0 NUMBER'
		$$ = new_attr(PIL_AFFINE,
			1.0,0.0,0.0,
			0.0,1.0, $3,
			0.0,0.0,1.0
		);
	}
	| IDENTIFIER ':' objectbody { // Child object
		pil_attr_t *name = new_attr(PIL_NAME,$1);
		LL_APPEND(name,$3);
		$$ = new_attr(PIL_CHILD,name);
	}
	| IDENTIFIER ':' CLASS objectbody { // Object class
		pil_attr_t *name = new_attr(PIL_NAME,$1);
		LL_APPEND(name,$4);
		$$ = new_attr(PIL_CLASS,name);
	}
	| IDENTIFIER ':' IDENTIFIER objectbody { // Class instantiation
		pil_attr_t *name = new_attr(PIL_NAME,$1);
		LL_APPEND(name,$4);
		$$ = new_attr(PIL_INST,$3,name);
	}
	| IDENTIFIER ':' STATE objectbody {
		pil_attr_t *name = new_attr(PIL_NAME,$1);
		LL_APPEND(name,$4);
		$$ = new_attr(PIL_STATE,name);
	};

timespec:
	  NUMBER SECONDS { $$ = $1; };

length:
	  NUMBER { $$ = $1; }
	| NUMBER PX { $$ = $1; };

paintspec:
	  RGB NUMBER NUMBER NUMBER {
		$$ = new_paint(PIL_COLOR,$2,$3,$4,1.0);
	}
	| RGBA NUMBER NUMBER NUMBER NUMBER {
		$$ = new_paint(PIL_COLOR,$2,$3,$4,$5);
	};

event:
	  PRESS { $$ = EVENT_PRESS; }
	| RELEASE { $$ = EVENT_RELEASE; }; 

pathspec:
	  segment {
		$$ = $1;
	}
	| pathspec segment {
		$$ = $2;
		LL_APPEND($1,$2);
	};

segment:
	  CLOSE {
		$$ = new_seg(PIL_CLOSE);
	}
	| LINE points {
		$$ = new_seg(PIL_LINE);
		$$->data.line.numpoints = $2.count/2;
		$$->data.line.points = $2.buf;
	}
	| QBEZIER points {
		$$ = new_seg(PIL_QUAD_BEZIER);
		$$->data.quadbezier.numpoints = $2.count/2;
		$$->data.quadbezier.points = $2.buf;
	}
	| TO NUMBER NUMBER {
		$$ = new_seg(PIL_MOVE_TO);
		$$->data.moveto.x = $2;
		$$->data.moveto.y = $3;
	};

points: { $$.count = $$.buflen = 0, $$.buf = NULL; }
	| points NUMBER NUMBER {
		$$ = $1;
		if(!$$.buf) {
			$$.buflen = 16;
			$$.buf = calloc($$.buflen,sizeof *$$.buf);
		}

		if($$.buflen < $$.count + 2) {
			$$.buflen *= 2;
			$$.buf = realloc($$.buf,$$.buflen*sizeof *$$.buf);
		}

		$$.buf[$$.count++] = $2;
		$$.buf[$$.count++] = $3;
	};

valuespec:
	  BIGIMAGE STRING X FROM NUMBER TO NUMBER Y FROM NUMBER TO NUMBER {
		$$ = malloc(sizeof *$$);
		$$->text = $2;
		$$->type = PIL_BIGIMAGE;
		$$->scale[0] = 1.0/($7 - $5);
		$$->offset[0] = $5*$$->scale[0];
		$$->scale[1] = 1.0/($12 - $10);
		$$->offset[1] = $10*$$->scale[1];
	}
	| PRINTF STRING {
		$$ = malloc(sizeof *$$);
		$$->type = PIL_PRINTF;
		$$->text = $2;
	}
	| ROTATE FROM NUMBER AT NUMBER TO NUMBER AT NUMBER {
		$$ = malloc(sizeof *$$);
		$$->type = PIL_ROTATE;
		$$->scale[0] = ($7 - $3)/($9 - $5);
		$$->offset[0] = $3 - $5*$$->scale[0];
	};

objectbody:
	  '{' attributes '}' { LL_FIND_HEAD($$,$2); };

%%

// Clamps all values to [0,1]
inline double normf(double x) {
	return x < 0 ? 0 : x < 1 ? x : 1;
}

pil_paint_t *new_paint(pil_paint_type_t type, ...) {
	va_list ap;
	pil_paint_t *paint;

	paint = calloc(1,sizeof *paint);
	paint->type = type;

	va_start(ap,type);

	switch(type) {
	case PIL_COLOR: // (type, double r, g, b, a)
		paint->data.color.r = normf(va_arg(ap,double));
		paint->data.color.g = normf(va_arg(ap,double));
		paint->data.color.b = normf(va_arg(ap,double));
		paint->data.color.a = normf(va_arg(ap,double));
		break;

	case PIL_UNKNOWN_PAINT:
	default:
		free(paint);
		paint = NULL;
		break;
	}

	va_end(ap);

	return paint;
}

pil_seg_t *new_seg(pil_seg_type_t type) {
	pil_seg_t *seg = calloc(1,sizeof *seg);
	seg->type = type;
	return seg;
}

pil_attr_t *new_attr(pil_attr_type_t type, ...) {
	va_list ap;
	pil_attr_t *attr;

	attr = calloc(1,sizeof *attr);
	attr->type = type;

	va_start(ap,type);

	switch(type) {
	case PIL_AFFINE: // double r1c1, r1c2, ..., r3c3
		attr->data.affine[0][0] = va_arg(ap,double);
		attr->data.affine[0][1] = va_arg(ap,double);
		attr->data.affine[0][2] = va_arg(ap,double);

		attr->data.affine[1][0] = va_arg(ap,double);
		attr->data.affine[1][1] = va_arg(ap,double);
		attr->data.affine[1][2] = va_arg(ap,double);

		attr->data.affine[2][0] = va_arg(ap,double);
		attr->data.affine[2][1] = va_arg(ap,double);
		attr->data.affine[2][2] = va_arg(ap,double);
		break;

	case PIL_CHILD: // pil_attr_t *attrs
		attr->data.child = va_arg(ap,pil_attr_t *);
		break;

	case PIL_CLASS: // pil_attr_t *attrs
		attr->data.class = va_arg(ap,pil_attr_t *);
		break;

	case PIL_EDGE: // double width, pil_paint_t *paint
		attr->data.edge.width = va_arg(ap,double);
		attr->data.edge.paint = va_arg(ap,pil_paint_t *);
		break;

	case PIL_EVENT: // pil_event_type_t type, char *nextstate
		attr->data.event.type = va_arg(ap,pil_event_type_t);
		attr->data.event.nextstate = va_arg(ap,char *);
		attr->data.event.trigger = va_arg(ap,char *);
		attr->data.event.timeout
			= attr->data.event.type == EVENT_TIMEOUT
			? va_arg(ap,double) : -1;
		break;

	case PIL_FILL: // pil_paint_t *paint
		attr->data.fill.paint = va_arg(ap,pil_paint_t *);
		break;

	case PIL_INST: // char *class, pil_attr_t *attrs
		attr->data.inst.class = va_arg(ap,char *);
		attr->data.inst.attrs = va_arg(ap,pil_attr_t *);
		break;

	case PIL_NAME: // char *name
		attr->data.name = va_arg(ap,char *);
		break;

	case PIL_PATH: // pil_seg_t *path
		attr->data.path = va_arg(ap,pil_seg_t *);
		break;

	case PIL_STATE: // pil_attr_t *attrs
		attr->data.state = va_arg(ap,pil_attr_t *);
		break;

	case PIL_VALUE: // pil_value_t *value
		attr->data.value = va_arg(ap,pil_value_t *);
		break;

	case PIL_WINDOW: // int width, int height
		attr->data.window.width = va_arg(ap,double);
		attr->data.window.height = va_arg(ap,double);
		break;

	case PIL_UNKNOWN_ATTR:
	default:
		free(attr);
		attr = NULL;
		break;
	}

	va_end(ap);

	return attr;
}

void yyerror(const char *msg) {
	fprintf(stderr,"PIL: line %i: %s\n",pilline,msg);
}

