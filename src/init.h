#pragma once 

#define MAX_ENV 100
#define MAX_ALIAS 100

struct env_vars {
    char *keys[MAX_ENV];
    char *vals[MAX_ENV];
};

struct aliases {
    char *keys[MAX_ALIAS];
    char *vals[MAX_ALIAS];
};

int cur_envvar;
int cur_alias;

struct env_vars var_table;
struct aliases alias_table; 

int call(char** args, int n_args);