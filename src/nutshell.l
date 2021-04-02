%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
"\n"                            {return RET;}
" "|"\t"                        {return WS;}
"~"                             {return TILDE;}
"."                             {return DOT;}
".."                            {return DOT2;}
\"(.*?)\"                       {return QUOTE;}
"<"|">"|"\|"|"\\"|"&"|"\""      {return META;}
[^~. "\t<>\|\\&\nX][^ "\t<>\|\\&\nX]+             {yylval.val = strdup(yytext); return WORD;}
%%

