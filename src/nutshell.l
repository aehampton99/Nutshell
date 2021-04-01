%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
"\n"                            {return RET;}
\"(.*?)\"                       {return QUOTE;}
"<"|">"|"\|"|"\\"|"&"|"\""      {return META;}
[^ "\t<>\|\\&\n]+             {yylval.val = strdup(yytext); return WORD;}
%%

