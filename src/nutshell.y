%{
#include "init.h"
#include <stdio.h>
#include <string.h>

#define MAX_ARGS 100

char *args[MAX_ARGS]; // args array 
int N_ARGS = 0;

int yylex();
int yyparse();

void addArg(char* a);
char* concat(const char *s1, const char *s2);
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
%token WS
%token QUOTE 
%token TILDE
%token META
%token WORD

%type<val> WORD input args param tilde_replace 

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
    param {addArg($1);}
    | args WS param {addArg($3);}
    ;

param:
    WORD
    | tilde_replace
    // | dot_replace
    // | dot2_replace
    ;

tilde_replace:
    TILDE {$$ = var_table.vals[0];}
    | TILDE WORD {$$ = concat(var_table.vals[0], $2);}
    ;

%% 

void addArg(char* a) {
    args[N_ARGS] = a;
    N_ARGS++;
}

char* concat(const char *s1, const char *s2)
{   
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator

    for (int i = 0; i < len1; i++)
        result[i] = s1[i];
    strcat(result, s2);

    return result;
}