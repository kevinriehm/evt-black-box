%{
#include <stdarg.h>
#include <stdlib.h>

#include "linked_list.h"
#include "pil.h"
%}

%union {
	char *string;
	double number;
	pil_seg_t *segment;
	pil_paint_t *paint;
	pil_attr_t *attribute;

	struct {
		int count;
		int buflen;
		double *buf;
	} numbers;
}

/* Keywords */
%token CLASS
%token EDGE FILL PATH
%token RGB RGBA
%token LINE

/* Etc. */
%token <string>   IDENTIFIER
%token <number>   NUMBER

/* Nonterminal types */
%type <segment>   pathspec segment
%type <paint>     paintspec
%type <attribute> attribute attributes
%type <numbers>   points

%{
void yyerror(const char *msg);
%}

%{
pil_paint_t *new_paint(pil_paint_type_t type, ...);
pil_seg_t *new_seg(pil_seg_type_t type);
pil_attr_t *new_attr(pil_attr_type_t type);


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
	  EDGE ':' paintspec {
		$$ = new_attr(PIL_EDGE);
		$$->data.paint = $3;
	}
	| FILL ':' paintspec {
		$$ = new_attr(PIL_FILL);
		$$->data.paint = $3;
	}
	| PATH ':' pathspec {
		$$ = new_attr(PIL_PATH);
		$$->data.path = $3;
	};

paintspec:
	  RGB NUMBER NUMBER NUMBER {
		$$ = new_paint(PIL_COLOR,$2,$3,$4,1.0);
	}
	| RGBA NUMBER NUMBER NUMBER NUMBER {
		$$ = new_paint(PIL_COLOR,$2,$3,$4,$5);
	};

pathspec: segment {
		$$ = $1;
	}
	| pathspec segment {
		$$ = $2;
		LL_APPEND($1,$2);
	};

segment:
	  LINE points {
		$$ = new_seg(PIL_LINE);
		$$->data.line.numpoints = $2.count/2;
		$$->data.line.points = $2.buf;
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
	case PIL_COLOR: // (type, double r, double g, double b, double a)
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

pil_attr_t *new_attr(pil_attr_type_t type) {
	pil_attr_t *attr = calloc(1,sizeof *attr);
	attr->type = type;
	return attr;
}

void yyerror(const char *msg) {
	fprintf(stderr,"PIL: line %i: %s\n",pilline,msg);
}

