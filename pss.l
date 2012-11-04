	#include "pss.h"
	#include "pss.tab.h"
	
	// Play nice with other lexers
	#define yylex  psslex
	#define yywrap psswrap
	#define yylval psslval
	
	double parse_num(char *str);

DEC [0-9]
HEX [0-9a-fA-F]
INT ({DEC}+|0x{HEX}+)
FLT ({DEC}+\.{DEC}*|{DEC}*\.{DEC}+)
NUM ({INT}|{FLT})

%%

[a-zA-Z_$-][a-zA-Z0-9_$-]* {
	yylval.string = calloc(strlen(yytext),sizeof(*yylval.string));
	strcpy(yylval.string,yytext);
	return IDENTIFIER;
}

{NUM} yylval.number = parse_num(yytext); return NUMBER;

[ \n]+ ;

. return yytext[0];

%%

double parse_num(char *str) {
	if(strncmp(str,"0x",2) == 0) { // Hexadecimal?
		unsigned int i;
		sscanf(str,"%x",&i);
		return (double) i;
	} else { // Float-format compatible
		return atof(str);
	}
}
