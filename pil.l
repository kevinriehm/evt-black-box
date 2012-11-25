 #include <stdlib.h>
 #include <string.h>

 #include "pil.h"
 #include "pil.tab.h"
 
 
 #define yyin pilin
 #define yylex pillex
 #define yylval pillval

D [0-9]
I {D}+
F ({I}.{I}?|{I}?.{I})([Ee][-+]?{I})?

%x BIGCOMMENT

%%

class return CLASS;

edge return EDGE;
fill return FILL;
path return PATH;

rgb return RGB;

line return LINE;

[a-zA-Z_$-][a-zA-Z0-9_$-]* yylval.string = strdup(yytext); return IDENTIFIER;
{F}                        yylval.number = atof(yytext); return NUMBER;

"//".*$           ; // Single line comment
"/*"              BEGIN BIGCOMMENT; // Multi-line comment
<BIGCOMMENT>.     ;
<BIGCOMMENT>"*/"  BEGIN INITIAL;

[ \n]+ ; // Whitespace merely separate tokens

. return yytext[0]; // Anything else passes through

%%

int yywrap() {
	return 1;
}

