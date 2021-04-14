%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
" "|"\t"|" \\\n"                                 {return WS;}
"\n"                                             {return RET;}
"~"                                              {return TILDE;}
"|"|" | "                                              {return PIPE;}
\$\{(.*?)\}                                      {yylval.val = strdup(yytext); return ENV;}
\"(.*?)\"                                        {yylval.val = strdup(yytext); return QUOTE;}
"<"|">"|"\|"|"\\"|"&"                            {return META;}
[^~ "\t<>\|\\&\nX][^ "\t<>\|\\&\nX]*             {yylval.val = strdup(yytext); return WORD;}
%%

