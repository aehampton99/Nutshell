%{
#include "init.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h> 
#include <fnmatch.h>

#define MAX_ARGS 100
#define MAX_PIPES 100
char *pipes[MAX_PIPES][MAX_ARGS]; // pipes
int n_per_pipe[MAX_PIPES];
int N_PIPES = 0;

char *args[MAX_ARGS]; // args array 
int N_ARGS = 0;

int err_redirect = 0;
int N_io = 0;
int is_piped = 0;

int yylex();
int yyparse();

void printargs();
void print_pipe_args();
void setup_pipe_input();
void io_redirection_no_pipes();
void io_redirection_pipes();
void handle_wild(char* w);
void alias_expansion();
char** list_files(char* pattern);
int filecmp(const void* s, const void* t);
void addArg(char* a);
void addEnvVarArg(char* a);
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
%token ERRREDIRECT
%token NEW
%token AMP
%token WILDFRONT
%token WILDBACK

%type<val> WORD QUOTE input args param tilde_replace remove_quote ENV env_var REDIRECT WILDFRONT WILDBACK

%%

input:
    %empty
    | input args whitespace background whitespace RET {
        alias_expansion();
        int ret = call(args+1, N_ARGS-1);
        N_ARGS = 0;
        AMPERSAND = 0;
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    | input pipe whitespace background whitespace RET {
        alias_expansion();
        setup_pipe_input();
        N_ARGS = 0;
        N_PIPES = 0;
        AMPERSAND = 0;
        memset(pipes, 0, sizeof(pipes));
        memset(n_per_pipe, 0, sizeof(n_per_pipe));
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    | input io_redirection whitespace background whitespace RET {
        // printf("FOUND IO REDIRECT\n");
        // printargs();
        alias_expansion();
        if(!is_piped) {
            io_redirection_no_pipes();
        } else {
            io_redirection_pipes();
        }
        N_io = 0;
        is_piped = 0;
        err_redirect = 0;
        N_ARGS = 0;
        N_PIPES = 0;
        AMPERSAND = 0;
        memset(pipes, 0, sizeof(pipes));
        memset(n_per_pipe, 0, sizeof(n_per_pipe));
        memset(args, 0, sizeof(args));
        YYACCEPT;}
    ;

background:
    %empty
    | AMP {AMPERSAND = 1;}
    ;

pipe:
    args whitespace PIPE whitespace args whitespace
    | pipe whitespace PIPE whitespace args whitespace
    ;

io_redirection:
    args whitespace REDIRECT whitespace WORD {addArg($3); addArg($5); N_io++;}
    | pipe whitespace REDIRECT whitespace WORD {addArg($3); addArg($5); N_io++; is_piped++;}
    | io_redirection whitespace REDIRECT whitespace WORD {addArg($3); addArg($5); N_io++;}
    | args whitespace ERRREDIRECT {err_redirect = 1;}
    | pipe whitespace ERRREDIRECT {err_redirect = 1;}
    | io_redirection whitespace ERRREDIRECT {err_redirect = 1;}
    ;

    whitespace:
    %empty
    | WS
    | NEW
    | whitespace WS
    | whitespace NEW
    ;

args:
    param {addArg("|"); addArg($1);}
    | args whitespace param {addArg($3);}
    | env_var {addArg("|"); addEnvVarArg($1);}
    | args whitespace env_var {addEnvVarArg($3);}
    | WILDFRONT {
        int matcher = $1[0] != '*';
        handle_wild($1+1, matcher);}
    | args whitespace WILDFRONT {
        int matcher = $3[0] != '*';
        handle_wild($3+1, matcher);}
    | WILDBACK {
        char* w = $1;
        int matcher = w[strlen($1)-1] != '*';
        w[strlen(w)-1] = '\0';
        handle_wild(w, matcher);}
    | args whitespace WILDBACK {
        char* w = $3;
        int matcher = w[strlen($3)-1] != '*';
        w[strlen(w)-1] = '\0';
        handle_wild(w, matcher);}
    ;

param:
    WORD
    | remove_quote
    | tilde_replace
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

void addEnvVarArg(char* a) {
    char *str = strdup(a);
    char* delim = " ";
    char *ptr = strtok(str, delim);

    while(ptr != NULL) {
        args[N_ARGS++] = ptr;
        ptr = strtok(NULL, delim);
    }

}

char* getEnv(char* s) {
    s[strlen(s)-1] = '\0';
    char* varName = s + 2;
    char* w = "";

    for (int i = 0; i < MAX_ENV; i++) {
        if (var_table.occupied[i] == 1 && strcmp(varName, var_table.keys[i]) == 0) {
            w = var_table.vals[i];
            break;
        }
    } 

    return w;
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

    // first index is always "|"
    for(int i = 1; i < N_ARGS; i++) {
        if (strcmp(args[i], "|") == 0) {
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
    if (err_redirect) {
        addArg("2>&1");
    }
    addArg(NULL);
    int piping = 0;
    redirection(args+1, N_ARGS-2, 0, NULL, 0, 0);
}

void io_redirection_pipes() {
    int idx = 0;
    int cur_pipe = 0;

    // first index is always "|"
    for(int i = 1; i < (N_ARGS-2*N_io)-err_redirect; i++) {
        if (strcmp(args[i], "|") == 0) {
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

    printf("PRINTING ARGS PER PIPE\n");
    for (int i = 0; i < N_PIPES; i++) {
        printf("Pipe %d\n", i);
        for (int j = 0; j <= n_per_pipe[i]; j++) {
            printf("%s\n", cmds[i][j]);
        }
        printf("\n");
    }

    // io redirect
    char* io_args[2*N_io+1+err_redirect];

    idx = 0;
    for(int i = N_ARGS-2*N_io; i < N_ARGS; i++) {
        io_args[idx++] = args[i];
    }
    if(err_redirect) {
        io_args[idx++] = "2>&1";
    }
    io_args[idx] = NULL;

    printf("PRINTING ARGS ARRAY\n");
    for(int i = 0; i < 2*N_io+1+err_redirect; i++) {
        printf("%s\n", io_args[i]);
    }

    printf("\n N_ARGS: %d \n", 2*N_io+err_redirect);
    printf("N_CMDS: %d\n", N_PIPES);
    
    redirection(io_args, 2*N_io+err_redirect, 1, cmds, n_per_pipe, N_PIPES);
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

int filecmp(const void* s, const void* t) {
    const char* ss = *(const char**)s;
    const char* tt = *(const char**)t;
    return strcmp(ss,tt);
}

char** list_files(char* pattern){
    char* filenames[MAX_FILES];

    DIR* dir;
    struct dirent *fl;
    char cwd[300];
    getcwd(cwd, sizeof(cwd));
    //printf("Executing in %s: \n", cwd);

    if ((dir = opendir(cwd)) != NULL) {
        int flnum = 0;
        while ((fl = readdir(dir)) != NULL) {
            char* fname = fl->d_name;

            if (fnmatch(pattern, fname, 0) == 0){
                filenames[flnum++] = fname;
            }
        }
        filenames[flnum] = NULL;

        closedir (dir);

        if (flnum > 1){
            qsort(filenames, flnum, sizeof(filenames[0]), filecmp);
        }

        // for (int i = 0; i < flnum; i++){
        //     printf("File %d in matching files: %s\n", i, filenames[i]);
        // }

        return filenames;
    } else {
        printf("Could not open directory.");
        return EXIT_FAILURE;
    }
}

char* str_slice(char* str, int start, int end){
    int length = end - start;
    char* out[length];

    for (int i = start; i <= end; i++) {
        out[i] = str[i];
    }

    out[length+1] = 0;

    return out;
}

void handle_wild(char* w) {
    char* matches = list_files(w);

    int i = 0;
    while(matches[i]) {
        if(strcmp(matches[i], ".") != 0 && strcmp(matches[i], "..") !=0)
            addArg(matches[i]);
        i++;
    }
 }

void alias_expansion() {
    char* w = args[1];
    int found = 1;
    
    while(found == 1) {
        found = 0;
        // check if w is in keys
        for (int i = 0; i < MAX_ALIAS; i++) {
            if (alias_table.occupied[i] == 1 && strcmp(w, alias_table.keys[i]) == 0) {
                w = alias_table.vals[i];
                found = 1;
                break;
            }
        }
    }

    args[1] = w;

    // split by spaces
    char *new_args[MAX_ARGS];
    new_args[0] = "|";
    char *str = strdup(w);
    char* delim = " ";
    char *ptr = strtok(str, delim);
    int idx = 1;

    while(ptr != NULL) {
        new_args[idx++] = ptr;
        ptr = strtok(NULL, delim);
    }

    int alias_words = idx-1;
    int new_n_args = N_ARGS+alias_words-1;
    for(int i = idx; i < new_n_args; i++) {
        new_args[i] = args[i - alias_words+1];
    }

    memset(args, 0, sizeof(args));
    N_ARGS = 0;
    for(int i = 0; i < new_n_args; i++) {
        addArg(new_args[i]);
    }
}