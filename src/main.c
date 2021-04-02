#include "init.h"
#include "nutshell.tab.h"
#include <stdio.h>
#include <string.h>

int yylex();
extern char* yytext;
extern int yyparse();

void init();
void setenv(char** args, int n_args);
void unsetenv(char** args, int n_args);
void printenv(char** args, int n_args);
void cd(char** args, int n_args);
void alias(char** args, int n_args);
void unalias(char** args, int n_args); 

int main() {
    init();
    printf("Nutshell\n");

    while(1) {
        printf(">");
        if (yyparse() == 1)
            break;
    }
    printf("...done\n");

    // while(1) {
    //     int token = yylex();
    //     if (token == 0) break;

    //     if (token == BUILT_IN) {
    //         printf("BUILT_IN\n");
    //         printf("%s\n", yytext);
    //     } else if (token == DOT) {
    //         printf("DOT\n");
    //         printf("%s\n", yytext);
    //     } else if (token == DOT2) {
    //         printf("DOT2\n");
    //         printf("%s\n", yytext);
    //     } else if (token == TILDE) {
    //         printf("TILDE\n");
    //         printf("%s\n", yytext);
    //     } else if (token == META) {
    //         printf("META\n");
    //         printf("%s\n", yytext);
    //     } else if (token == WORD) {
    //         printf("WORD\n");
    //         printf("%s\n", yytext);
    //     } else if (token == QUOTE) {
    //         printf("QUOTE\n");
    //         printf("%s\n", yytext);
    //     } else if (token == WS) {
    //         printf("WS\n");
    //         printf("%s\n", yytext);
    //     }
    // }

}

void init() {
    for (int i = 0; i < MAX_ENV; i++) {
        var_table.occupied[i] = 0;
    }

    for (int i = 0; i < MAX_ALIAS; i++) {
        alias_table.occupied[i] = 0;
    }

    // set HOME
    var_table.occupied[0] = 1;
    var_table.keys[0] = "HOME";
    var_table.vals[0] = "home"; // needs legit default value 
    HOME = var_table.vals[0]; // make sure this is shallow copy 

    // set PATH
    var_table.occupied[1] = 1;
    var_table.keys[1] = "PATH";
    var_table.vals[1] = "path"; // needs legit default value 
    PATH = var_table.vals[1];
}

// main entry point for a command 
int call(char** args, int n_args) {
    printf("Printing Args in Main.C\n");
    for (int i = 0; i < n_args; i++) {
        printf("%s\n", args[i]);
    }

    // TODO: check for aliases first 
    // check for built in command 
    char* cmd = args[0];

    if (strcmp(cmd, "setenv") == 0) {
        setenv(args, n_args);
    } else if (strcmp(cmd, "printenv") == 0) {
        printenv(args, n_args);
    } else if (strcmp(cmd, "unsetenv") == 0) {
        unsetenv(args, n_args);
    } else if (strcmp(cmd, "cd") == 0) {
        cd(args, n_args);
    } else if (strcmp(cmd, "alias") == 0) {
        alias(args, n_args);
    } else if (strcmp(cmd, "unalias") == 0) {
        unalias(args, n_args);
    } else if (strcmp(cmd, "bye") == 0) {
        return 1;
    } else {
        // extern 
    }

    return 0;

}

void setenv(char** args, int n_args) {
    //
    printf("SET ENV %s = %s\n", args[1], args[2]);

    if (n_args != 3) {
        printf("ERROR: EXPECTED 2 ARGUMENTS, GOT %d\n", n_args-1);
        return;
    }

    // check if currently in the table 
    for (int i = 0; i < MAX_ENV; i++) {
        if(var_table.occupied[i] == 1 
            && strcmp(args[1], var_table.keys[i]) == 0) {
            var_table.vals[i] = args[2];
            return;
        }
    }

    // if not in table, put into the first unoccupied space 
    for (int i = 0; i < MAX_ENV; i++) {
        if (var_table.occupied[i] == 0) {
            var_table.keys[i] = args[1];
            var_table.vals[i] = args[2];
            var_table.occupied[i] = 1;
            printf("entered at index %d\n", i);
            return;
        }
    }

    printf("ERROR: MAXIMUM ENVIRONMNET VARIABLES REACHED\n");
}

void printenv(char** args, int n_args) {
    if (n_args > 1) {
        printf("WARNING: EXPECTED 0 ARGUMENTS, GOT %d\n", n_args-1);
    }

    // TODO print PATH and HOME

    for (int i = 0; i < MAX_ENV; i++) {
        if(var_table.occupied[i]) {
            printf("%s = %s\n", var_table.keys[i], var_table.vals[i]);
        }
    }

}

void cd(char** args, int n_args) {

    // verify args
    if (n_args == 1) {
        printf("ERROR: EXPECTED 1 ARGUMENT, GOT 0\n");
        return;
    } else if (n_args > 2) {
        printf("WARNING: EXPECTED 1 ARGUMENT, GOT %d\n", n_args-1);
    }

    // TODO: wildcard matching 
}

void alias(char** args, int n_args) { 
    //
}

void unalias(char** args, int n_args) {
    //
}
