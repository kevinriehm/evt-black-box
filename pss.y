%{
	pss_block_t *pssroot = NULL;
%}

%union {
	char *string;
	double number;
	pss_block_t *block;
	pss_attr_t *attribute;
}

%token <string> IDENTIFIER

%type <block> blocks block
%type <attribute> attributes attribute

%%

root: blocks {
		if(pssroot = $1)
			while(pssroot->prev)
				pssroot = pssroot->prev;
	};

blocks: {
		$$ = NULL;
	}
	| blocks block {
		$$ = $2;
		
		if($1) {
			$$->prev = $1;
			$1->next = $$;
		}
	}

block: selector '{' attributes '}' {
		$$ = calloc(1,sizeof(*$$));
	};

selector: ;

attributes: {
		$$ = NULL;
	}
	| attributes attribute {
		$$ = $2;
		
		if($1) {
			$$->prev = $1;
			$1->next = $$;
		}
	};

attribute: IDENTIFIER ':' value ';' {
		$$ = calloc(1,sizeof(*$$));
	};

value: ;

%%


