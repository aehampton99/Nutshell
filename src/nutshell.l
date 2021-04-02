%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
"\n"                            {return RET;}
" "|"\t"                        {return WS;}
"~"                             {return TILDE;}
\"(.*?)\"                       {return QUOTE;}
"<"|">"|"\|"|"\\"|"&"|"\""      {return META;}
[^~ "\t<>\|\\&\nX][^ "\t<>\|\\&\nX]*             {yylval.val = strdup(yytext); return WORD;}
%%

