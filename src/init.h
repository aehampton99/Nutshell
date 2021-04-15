#pragma once 

#define MAX_ENV 100
#define MAX_ALIAS 100
#define MAX_PATH_CHAR 300

struct env_vars {
    int occupied[MAX_ENV];
    char *keys[MAX_ENV];
    char *vals[MAX_ENV];
};

struct aliases {
    int occupied[MAX_ALIAS];
    char *keys[MAX_ALIAS];
    char *vals[MAX_ALIAS];
};

int cur_envvar;
int cur_alias;
int AMPERSAND;

struct env_vars var_table;
struct aliases alias_table; 
int BYE;

char *PATH;
char *HOME;

int call(char** args, int n_args);
void piped(char*** cmds, int* n_cmd_args, int n_cmds);
void redirection(char** args, int n_args, int piping, char*** cmds, int *n_cmd_args, int n_cmds);
// void piped();
// void redirection();