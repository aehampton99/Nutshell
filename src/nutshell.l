%{
    #include "init.h"
    #include "nutshell.tab.h"
    #include <string.h>
%}                         

%%
[ \t]+                                                          {return WS;}
"\\\n"                                                          {return NEW;}
"\n"                                                            {return RET;}
"~"                                                             {return TILDE;}
"|"|" | "                                                       {return PIPE;}
"<"|" < "|">"|" > "|">>"|" >> "|"2>"|" 2> "                     {yylval.val = strdup(yytext);return REDIRECT;}
"2>&1"|" 2>&1 "|" 2>&1"                                         {return ERRREDIRECT;}   
\$\{(.*?)\}                                                     {yylval.val = strdup(yytext); return ENV;}
\"(.*?)\"                                                       {yylval.val = strdup(yytext); return QUOTE;}
"<"|">"|"\|"|"\\"|"&"                                           {return META;}
[^~ "\t<>\|\\&\nX][^ "\t<>\|\\&\nX]*                            {yylval.val = strdup(yytext); return WORD;}
%%

