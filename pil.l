 #include <stdlib.h>
 #include <string.h>

 #include "pil.h"
 #include "pil.tab.h"
 
 
 #define yyin pilin
 #define yylex pillex
 #define yylval pillval
 
 
 int pilline = 1;

D [0-9]
I -?{D}+
F -?({D}+\.{D}*|{D}*\.{D}+)([Ee][+-]?{D}+)?
N ({I}|{F})

%x BIGCOMMENT

%%

class     return CLASS;
state     return STATE;

edge      return EDGE;
fill      return FILL;
path      return PATH;
rotate    return ROTATE;
scale     return SCALE;
shear     return SHEAR;
translate return TRANSLATE;
window    return WINDOW;
x         return X;
y         return Y;

press     return PRESS;

px        return PX;

rgb       return RGB;
rgba      return RGBA;

line      return LINE;

{N}                        yylval.number = atof(yytext); return NUMBER;
[a-zA-Z_$-][a-zA-Z0-9_$-]* yylval.string = strdup(yytext); return IDENTIFIER;

"//".*$           ; // Single line comment
"/*"              BEGIN BIGCOMMENT; // Multi-line comment
<BIGCOMMENT>.     ;
<BIGCOMMENT>"*/"  BEGIN INITIAL;

[ \t]+ ; // Whitespace merely separates tokens
\n   pilline++; // Track line numbers for error reporting

. return yytext[0]; // Anything else passes through

%%

int yywrap() {
	return 1;
}

