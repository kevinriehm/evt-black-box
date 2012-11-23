 #include "pil.h"
 #include "pil.tab.h"
 
 #define yylex pillex
 #define yytext piltext

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

[a-zA-Z_$-][a-zA-Z0-9_$-]* yylval.string = strdup(yytext); return IDENTIFIER;
{F}                        yylval.number = atod(yytext); return NUMBER;

"//".*$           ; // Single line comment
"/*"              BEGIN BIGCOMMENT; // Multi-line comment
<BIGCOMMENT>.     ;
<BIGCOMMENT>"*/"  BEGIN INITIAL;

[ \n]+ ; // Whitespace merely separate tokens

. return yytext[0]; // Anything else passes through

%%

char *strdup(char *str) {
	char *newstr = calloc(sizeof *str,strlen(str));
	return strcpy(newstr,str);
}

