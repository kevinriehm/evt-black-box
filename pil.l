#define yylex pillex
#define yytext piltext

%%

/\*\*/|//$ ; // Ignore comments

 * ; // Spaces and tabs merely separate tokens

. return yytext[0]; // Anything else passes through

