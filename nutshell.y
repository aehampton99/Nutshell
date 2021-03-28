%{
#include <stdio.h>

int yylex(); // Defined in lex.yy.c

int yyparse(); // Need this definition so that yyerror can call it

void yyerror(char* e) {
	printf("Error: %s\n", e);

	// NOTE: calling yyparse from within yyerror has consequences, and I only
	// realized this after I held and recorded the tutorial.

	// Since yyparse calls yyerror, if you call yyparse again then you're
	// recursing into yyerror and eating up stack space. This could lead to a
	// seg fault if called enough times. Instead, it's smarter to call yyparse
	// exclusively from your main method, and use yyerror to recover from and
	// report errors.

	// Leaving this in here to show it as a very simple example.

	yyparse();
}
%}

%token BUILTIN, META, WORD, FLAG, WHITESPACE, UNDEFINED

