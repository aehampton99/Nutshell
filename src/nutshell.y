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
char* getEnv(char* s);
void yyerror(char* e) {
    N_ARGS = 0;
    memset(args, 0, sizeof(args));
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
%token ENV
%token META
%token WORD

%type<val> WORD QUOTE input args param tilde_replace remove_quote ENV env_var

%%

input:
    %empty
    | input args RET{
<<<<<<< HEAD
        printf("Calling\n");
        call(args, N_ARGS);
=======
        //printf("Calling\n");
        int ret = call(args, N_ARGS);
>>>>>>> pipes
        N_ARGS = 0;
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    ;

args:
    param {addArg($1);}
    | args WS param {addArg($3);}
    ;

param:
    WORD
    | remove_quote
    | tilde_replace
    | env_var
    ;

tilde_replace:
    TILDE {$$ = var_table.vals[0];}
    | TILDE WORD {$$ = concat(var_table.vals[0], $2);}
    ;

remove_quote: 
    QUOTE {$$[strlen($1)-1] = '\0'; $$ = $1 + 1; printf("%s\n", $$);}
    ;

env_var:
    ENV {
        char* val = getEnv($1);
        if(strlen(val) == 0) {
            yyerror("VARIABLE NOT FOUND");
            YYABORT;
        }
        $$ = val; }
    ;

%% 

void addArg(char* a) {
    args[N_ARGS] = a;
    N_ARGS++;
}

char* getEnv(char* s) {
    s[strlen(s)-1] = '\0';
    char* varName = s + 2;

    for (int i = 0; i < MAX_ENV; i++) {
        if (var_table.occupied[i] == 1 && strcmp(varName, var_table.keys[i]) == 0) {
            return var_table.vals[i];
        }
    }
    return "\0";
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