%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
"\n"                            {return RET;}
\"(.*?)\"                       {return QUOTE;}
"."                             {return DOT;}
".."                            {return DOT2;}
"~"                             {return TILDE;}
"<"|">"|"\|"|"\\"|"&"|"\""      {return META;}
[^ "\t<>\|\\&\n.~]+             {yylval.val = strdup(yytext); return WORD;}
%%

