%{
#include "init.h"
#include <stdio.h>

#define MAX_ARGS 100

char *args[MAX_ARGS]; // args array 
int N_ARGS = 0;

int yylex();
int yyparse();

void addArg(char* a);
void yyerror(char* e) {
    printf("Error: %s\n", e);
}
%}

%union {
    char *val;
    int id; 
}

%start input

%token RET
%token QUOTE 
%token DOT
%token DOT2
%token TILDE
%token META
%token WORD

%type<val> WORD input args

%%

input:
    %empty
    | input args RET{
        printf("Calling\n");
        int ret = call(args, N_ARGS);
        N_ARGS = 0;
        if (ret)
            YYABORT;
        YYACCEPT;}
    ;

args:
    WORD {addArg($1);}
    | args WORD {addArg($2);}
    ;

%% 

void addArg(char* a) {
    args[N_ARGS] = a;
    N_ARGS++;
}

