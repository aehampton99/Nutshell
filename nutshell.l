%{
    #include "nutshell.tab.h"
%}

%%
\"(.*?)\"                                           yylval.val = yytext; return QUOTE;
setenv|printenv|unsetenv|cd|alias|unalias		    yylval.val = yytext; return BUILTIN;
bye                                                 yylval.val = yytext; return EXIT;
"\\"|"["|"]"|"~"|"<"|">"					        yylval.val = yytext; return META;
"."                                                 yylval.val = yytext; return DOT;
".."                                                yylval.val = yytext; return DOTDOT;
[^ "\t<>\|\\&\n.~]+					                yylval.val = yytext; return WORD;
%%