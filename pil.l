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

after     return AFTER;
edge      return EDGE;
fill      return FILL;
on        return ON;
path      return PATH;
rotate    return ROTATE;
scale     return SCALE;
shear     return SHEAR;
translate return TRANSLATE;
value     return VALUE;
window    return WINDOW;
x         return X;
y         return Y;

press     return PRESS;
release   return RELEASE;
seconds   return SECONDS;

px        return PX;

rgb       return RGB;
rgba      return RGBA;

close     return CLOSE;
line      return LINE;
qbezier   return QBEZIER;
to        return TO;

printf    return PRINTF;
from      return FROM;
at        return AT;


{N}                        yylval.number = atof(yytext); return NUMBER;
[a-zA-Z_$-][a-zA-Z0-9_$-]* yylval.string = strdup(yytext); return IDENTIFIER;
\"(\\.|[^\"])*\"           {
	yylval.string = strdup(yytext + 1);
	yylval.string[strlen(yylval.string) - 1] = '\0';
	return STRING;
}

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

