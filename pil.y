%{
#include "linked_list.h"
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
%token RGB
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
pil_color_t *new_color(double r, double g, double b);
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
		$$ = new_color($2,$3,$4);
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
			$$.buf = realloc($$.buflen*sizeof *$$.buf);
		}

		$$.buf[$$.count++] = $2;
		$$.buf[$$.count++] = $3;
	};

%%

// Clamps all values to [0,1]
inline double normf(double x) {
	return x < 0 ? 0 : x < 1 ? x : 1;
}

pil_color_t *new_color(double r, double g, double b) {
	pil_color_t *color = calloc(1,sizeof *color);
	color->r = normf(r);
	color->g = normf(g);
	color->b = normf(b);
	return color;
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

