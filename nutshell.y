%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int yylex();
extern int yyparse(); 
extern char* yytext;
extern FILE* yyin;
extern FILE* yyout;


void yyerror(char* e) {
	printf("Error: %s\n", e);
}
%}

%union{
    int id;
    char* val;
}

%token BUILTIN
%token META 
%token WORD 
%token WHITESPACE 
%token QUOTE 
%token DOT 
%token DOTDOT
%start input
%type<val> BUILTIN WORD args input

%%
input:
    %empty
    | input args {printf("Calling functions %s\n", $2);}
    ;

args:
     WORD
    | BUILTIN
;


