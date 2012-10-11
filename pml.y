%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	
	#include "pml.h"
	
	pml_node_t *pmlroot = NULL; // The end result of parsing
	
	pml_node_t *create_node(char *type);
	pml_attr_t *create_attribute(char *type, char *value);
%}

%union {
	char *string;
	char character;
	pml_node_t *node;
	pml_attr_t *attribute;
}

%nonassoc UNIMPORTANT
%token <string> SIMPLETAGNAME COMPLEXTAGNAME IDENTIFIER QUOTEDTEXT
%nonassoc <character> CHARACTER

%type <string> text
%type <node> tag tags tagcontent
%type <attribute> attribute attributes

%%

root: tags {
		if(pmlroot = $1)
			while(pmlroot->prev)
				pmlroot = pmlroot->prev;
	};

tags: {
		$$ = NULL;
	}
	| tags tag { // Append tags to tag
		$$ = $2;
		
		if($1) {
			$$->parent = $1->parent;
			
			$$->prev = $1;
			$1->next = $$;
		}
	};

tag:
	  '<' SIMPLETAGNAME attributes '>' {
		$$ = create_node($2);
		$$->attrs = $3;
	}
	| '<' COMPLEXTAGNAME attributes '>' tagcontent '<' '/' COMPLEXTAGNAME '>' {
		pml_node_t *child = $5;
		
		if(strcmp($2,$8) != 0) {
			yyerror("closing tag mismatch");
			YYABORT;
		}
		
		$$ = create_node($2);
		
		// Add the attributes
		if($$->attrs = $3)
			while($$->attrs->prev)
				$$->attrs = $$->attrs->prev;
		
		// Add the children
		if($$->children = child)
			while($$->children->prev)
				$$->children->parent = $$,
				$$->children = $$->children->prev;
	};

tagcontent: {
		$$ = NULL;
	}
	| tagcontent text %prec UNIMPORTANT {
		$$ = create_node("text");
		
		$$->text = $2;
		
		if($1) {
			$$->prev = $1;
			$1->next = $$;
		}
	}
	| tagcontent tag {
		$$ = $2;
		
		if($1) {
			$$->prev = $1;
			$1->next = $$;
		}
	};

attributes: {
		$$ = NULL;
	}
	| attributes attribute { // Append attributes to attribute
		$$ = $2;
		
		if($1) {
			$$->prev = $1;
			$1->next = $$;
		}
	};

attribute:
	  IDENTIFIER {
		$$ = create_attribute($1,NULL);
	}
	| IDENTIFIER '=' QUOTEDTEXT {
		$$ = create_attribute($1,$3);
	};

text: CHARACTER {
		$$ = calloc(16,sizeof(char *));
		$$[15] = -1;
		$$[0] = $1;
	}
	| text CHARACTER {
		$$ = $1;
		
		int len = strlen($$);
		
		// Does it need to be expanded?
		if($$[len + 1] == -1) {
			$$ = realloc($$,2*len*sizeof(char *));
			memset($$ + len,'\0',len);
			$$[2*len - 1] = -1;
		}
		
		$$[len] = $2;
	};

%%

pml_attr_t *create_attr(char *type) {
	pml_attr_t *attr;
}

pml_node_t *create_node(char *type) {
	pml_node_t *node;
	
	node = calloc(1,sizeof(*node));
	
	// Translate the node's type
	if(strcmp(type,"text") == 0) node->type = PMLNODE_TEXT;
	else node->type = PMLNODE_UNKNOWN;
	
	node->typestring = type;
	
	return node;
}

pml_attr_t *create_attribute(char *type, char *value) {
	pml_attr_t *attr;
	
	attr = calloc(1,sizeof(*attr));
	
	attr->hasvalue = !!value; // Valuable?
	
	// Translate the attribute's type
	if(strcmp(type,"id") == 0) attr->type = PMLATTR_ID;
	else attr->type = PMLATTR_UNKNOWN;
	
	attr->typestring = type;
	
	if(value) attr->value = value;
	
	return attr;
}
/*
void indent(int level) {
	int i;
	for(i = 0; i < level; i++)
		putchar('\t');
}

void print_pml_tree(pml_node_t *root) {
	static int level = -1;
	
	pml_attr_t *attr;
	
	level++;
	
	while(root) {
		// Output some useful information
		
		indent(level);
		
		printf(root->typestring);
		
		for(attr = root->attrs; attr; attr = attr->next)
			printf("%c %s%s%s",attr->prev ? ',' : ':',
				attr->typestring,
				attr->hasvalue ? " = " : "",
				attr->hasvalue ? attr->value : "");
		
		if(root->text) printf(": %s",root->text);
		
		putchar('\n');
		
		// Recurse further into the endless abyss...
		print_pml_tree(root->children);
		
		root = root->next;
	}
	
	level--;
}

int main() {
	yyparse(); // Read the input
	print_pml_tree(pmlroot); // Describe what was seen
}
*/
int yyerror(const char *msg) {
	fprintf(stderr,"pml: %s\n",msg);
}
