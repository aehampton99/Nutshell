%{
#include "init.h"
#include <stdio.h>
#include <string.h>

#define MAX_ARGS 100
#define MAX_PIPES 100
char *pipes[MAX_PIPES][MAX_ARGS]; // pipes
int n_per_pipe[MAX_PIPES];
int N_PIPES = 0;

char *args[MAX_ARGS]; // args array 
int N_ARGS = 0;

int yylex();
int yyparse();

void printargs();
void print_pipe_args();
void setup_pipe_input();
void io_redirection_no_pipes();
void io_redirection_pipes();
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


%token RET
%token WS
%token QUOTE 
%token TILDE
%token ENV
%token META
%token WORD
%token PIPE
%token REDIRECT

%type<val> WORD QUOTE input args param tilde_replace remove_quote ENV env_var REDIRECT

%%

input:
    %empty
    | input args RET {
        int ret = call(args+1, N_ARGS-1);
        N_ARGS = 0;
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    | input pipe RET {
        setup_pipe_input();
        N_ARGS = 0;
        N_PIPES = 0;
        memset(pipes, 0, sizeof(pipes));
        memset(n_per_pipe, 0, sizeof(n_per_pipe));
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    | input io_redirection RET {
        // printf("FOUND IO REDIRECT\n");
        // printargs();
        io_redirection_no_pipes();
        N_ARGS = 0;
        N_PIPES = 0;
        memset(pipes, 0, sizeof(pipes));
        memset(n_per_pipe, 0, sizeof(n_per_pipe));
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    ;

pipe:
    args PIPE args 
    | pipe PIPE args 
    ;

io_redirection:
    args REDIRECT WORD {addArg($2); addArg($3);}
    | pipe REDIRECT WORD
    | io_redirection REDIRECT WORD
    ;

args:
    param {addArg(""); addArg($1);}
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
    QUOTE {$$[strlen($1)-1] = '\0'; $$ = $1 + 1;}
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

void printargs() {
    printf("PRINTING ARGS IN SEQUENCE\n");
    for (int i = 0; i < N_ARGS; i++) {
        printf("%s\n", args[i]);
    }
}

void setup_pipe_input() {
    int idx = 0;
    int cur_pipe = 0;

    // first index is always ""
    for(int i = 1; i < N_ARGS; i++) {
        if (strcmp(args[i], "") == 0) {
            pipes[cur_pipe][idx] = NULL;
            n_per_pipe[cur_pipe] = idx;
            cur_pipe++;
            N_PIPES++;
            idx = 0;
        } else {
            pipes[cur_pipe][idx++] = args[i];
        }
    }

    // info for the last pipe
    pipes[cur_pipe][idx] =  NULL;
    n_per_pipe[cur_pipe] = idx;
    N_PIPES++;

    char **cmds[N_PIPES+1];
    for (int i = 0; i < N_PIPES; i++) {
        cmds[i] = pipes[i];
    }
    cmds[N_PIPES] = NULL;

    // call piped 
    piped(cmds, n_per_pipe, N_PIPES);
}

void io_redirection_no_pipes() {
    addArg(NULL);
    int piping = 0;
    redirection(args+1, N_ARGS-2, 0, NULL, 0, 0);
    //redirection();
}

void print_pipe_args() {
    printf("PRINTING ARGS PER PIPE\n");
    for (int i = 0; i < N_PIPES; i++) {
        printf("Pipe %d\n", i);
        for (int j = 0; j <= n_per_pipe[i]; j++) {
            printf("%s\n", pipes[i][j]);
        }
        printf("\n");
    }
}