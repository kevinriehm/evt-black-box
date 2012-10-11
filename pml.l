	#include "pml.h"
	#include "pml.tab.h"
	
	// Play nice with other lexers
	#define yylex  pmllex
	#define yywrap pmlwrap
	#define yylval pmllval

%s OPENTAG CLOSETAG

%%

<INITIAL>< BEGIN OPENTAG; REJECT;
<OPENTAG>> BEGIN INITIAL; REJECT;

<INITIAL><\/ BEGIN CLOSETAG; REJECT;
<CLOSETAG>>  BEGIN INITIAL; REJECT;

<OPENTAG,CLOSETAG>button|p|span {
	yylval.string = calloc(strlen(yytext) + 1,sizeof(*yylval.string));
	strcpy(yylval.string,yytext);
	return COMPLEXTAGNAME;
}

<OPENTAG>img {
	yylval.string = calloc(strlen(yytext) + 1,sizeof(*yylval.string));
	strcpy(yylval.string,yytext);
	return SIMPLETAGNAME;
}

<OPENTAG,CLOSETAG>[a-zA-Z_-][a-zA-Z0-9_-]* {
	yylval.string = calloc(strlen(yytext) + 1,sizeof(*yylval.string));
	strcpy(yylval.string,yytext);
	return IDENTIFIER;
}

<OPENTAG>\"(\\.|[^\\"])*\" {
	yylval.string = calloc(strlen(yytext) - 1,sizeof(*yylval.string));
	strncpy(yylval.string,yytext + 1,strlen(yytext) - 2);
	return QUOTEDTEXT;
}

<INITIAL>[^<>] yylval.character = yytext[0]; return CHARACTER;

[ \n]+ ;

. return yytext[0];

%%

int yywrap() {
	return 1;
}
