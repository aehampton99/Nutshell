%{
    #include types.h
}$

%%
setenv|printenv|unsetenv|cd|alias|unalias|bye		yylval.val = yytext; return BUILTIN;
"."|".."|"\\"|"["|"]"|"~"					yylval.val = yytext; return META;
\"(.*?)\"                       yylval.val = yytext; return QUOTE;
[A-Za-z]* 						yylval.val = yytext; return WORD;
-[A-Za-z]*						yylval.val = yytext; return FLAG;
" "|"\t"                  yylval.val = yytext; return WHITESPACE;
.                               yylval.val = yytext; return UNDEFINED;

%%