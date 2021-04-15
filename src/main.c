#include "init.h"
#include "nutshell.tab.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h> 

int yylex();
extern char* yytext;
extern int yyparse();

void init();
void setenvir(char** args, int n_args);
void unsetenvir(char** args, int n_args);
void printenv(char** args, int n_args);
void cd(char** args, int n_args);
void alias(char** args, int n_args);
void unalias(char** args, int n_args);
void call_extern(char** args, int n_args);

int main() {
    init();
    printf("Nutshell\n");

    while(BYE == 0) {
        printf(">");
        yyparse();
    }
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
    char cwd[150];
    getcwd(cwd, sizeof(cwd));
    var_table.vals[0] = cwd; // needs legit default value 
    HOME = var_table.vals[0]; // make sure this is shallow copy 

    // set PATH
    var_table.occupied[1] = 1;
    var_table.keys[1] = "PATH";
    var_table.vals[1] = ".:/usr/bin:/bin"; // needs legit default value 
    PATH = var_table.vals[1];

    // set ampersand
    AMPERSAND = 0;

    // set BYE
    BYE = 0;
}

// main entry point for a command 
int call(char** args, int n_args) {
    char* cmd = args[0];

    if (strcmp(cmd, "setenv") == 0) {
        setenvir(args, n_args);
    } else if (strcmp(cmd, "printenv") == 0) {
        printenv(args, n_args);
    } else if (strcmp(cmd, "unsetenv") == 0) {
        unsetenvir(args, n_args);
    } else if (strcmp(cmd, "cd") == 0) {
        cd(args, n_args);
    } else if (strcmp(cmd, "alias") == 0) {
        alias(args, n_args);
    } else if (strcmp(cmd, "unalias") == 0) {
        unalias(args, n_args);
    } else if (strcmp(cmd, "bye") == 0) {
        BYE = 1;
    }else {
        call_extern(args, n_args); 
    }

    return 0;
}

void call_extern(char** args, int n_args) {
    // for each in path 
    char* path_copy[strlen(var_table.vals[1])];
    strcpy(path_copy, var_table.vals[1]);
    char* token;

    token = strtok(path_copy, ":");

    char* result[INT16_MAX];

    // fork
    pid_t p;

    p = fork();

    // check fork worked
    if (p < 0){
        printf("Fork failed.");
    } else if (p == 0){
        int worked = 0;

        while (token != NULL) {
            strcpy(result, token);
            strcat(result, "/");
            strcat(result, args[0]);

            if(execv(result, args) != -1){
                worked = 1;
                break;
            }
            else {
                worked = -1;
            }
            // go next 
            token = strtok(NULL, ":");
        }

        if (worked == -1){
            printf("Could not find command: %s\n", args[0]);
        }

        exit(0);
    }
    else {
        if (AMPERSAND == 0){
            wait(0);
        }
    }
}

void piped(char*** cmds, int* n_cmd_args, int n_cmds) {

    int fd[2], input;
    pid_t p;

    int i = 0;
    for (int i = 0; i < n_cmds; i++){
        pipe(fd);

        p = fork();

        if (p < 0){
            printf("Houston we have a problem.\n");
            exit(1);
            return;
        }
        else if (p == 0){
            dup2(input, STDIN_FILENO);
            close(input);

            if (cmds[i + 1] != NULL){
                dup2(fd[1], STDOUT_FILENO);
            }

            close(fd[0]);
            call(cmds[i], n_cmd_args[i]);
            exit(0);
        }
        else{
            wait(&p);

            input = fd[0];
            close(fd[1]);
        }
    }
}

void redirection(char** args, int n_args, int piping, char*** cmds, int* n_cmd_args, int n_cmds){

    int input = 0, output = 0, append = 0;
    char* commands[n_args];
    int c = 0;

    pid_t p;
    p = fork();

    if (p < 0){
        exit(1);
        return;
    } else if (p == 0){
        for (int i = 0; i < n_args; i++){
            int j;
            if (!strcmp(args[i], "<") || !strcmp(args[i], " < ")){
                ++i;
                input = open(args[i], O_RDONLY);

                dup2(input, STDIN_FILENO);
                close(input);
                continue;
            }
            if (!strcmp(args[i], ">") || !strcmp(args[i], " > ")){
                ++i;
                output = creat(args[i], 0644);

                dup2(output, STDOUT_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], ">>") || !strcmp(args[i], " >> ")){
                ++i;
                append = open(args[i], O_CREAT | O_RDWR | O_APPEND, 0644);

                dup2(append, STDOUT_FILENO);
                close(append);
                continue;
            }
            if (!strcmp(args[i], "2>") || !strcmp(args[i], " 2> ")){
                ++i;
                output = creat(args[i], 0644);

                dup2(output, STDERR_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], "2>&1") || !strcmp(args[i], " 2>&1 ")){
                ++i;
                dup2(STDOUT_FILENO, STDERR_FILENO);
                close(stdout);
                continue;
            }
            commands[c++] = args[i];
        }
        commands[c] = NULL;
        if (piping == 1){
            piped(cmds, n_cmd_args, n_cmds);
        }
        else{
            call(commands, c);
        }
        exit(0);
    } else {
        wait(&p);
    }
}

void setenvir(char** args, int n_args) {


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
            return;
        }
    }

    printf("ERROR: MAXIMUM ENVIRONMNET VARIABLES REACHED\n");
}

void unsetenvir(char** args, int n_args) {
    if (n_args > 2) {
        printf("WARNING: EXPECTED 1 ARGUMENT, GOT %d\n", n_args-1);
    } else if (strcmp(args[1], "HOME") == 0 || strcmp(args[1], "PATH") == 0) {
        printf("ERROR: CANNOT REMOVE MANDATORY VARIABLE\n");
        return;
    }

    for (int i = 0; i < MAX_ENV; i++) {
        if (var_table.occupied[i] == 1 && strcmp(args[1], var_table.keys[i]) == 0)
            var_table.occupied[i] = 0;
    }

}

void printenv(char** args, int n_args) {
    if (n_args > 1) {
        printf("WARNING: EXPECTED 0 ARGUMENTS, GOT %d\n", n_args-1);
    }

    for (int i = 0; i < MAX_ENV; i++) {
        if(var_table.occupied[i]) {
            printf("%s=%s\n", var_table.keys[i], var_table.vals[i]);
        }
    }

}

void cd(char** args, int n_args) {

    // verify args
    if (n_args == 1) {
        args[1] = var_table.vals[0];
    } else if (n_args > 2) {
        printf("WARNING: EXPECTED 1 ARGUMENT, GOT %d\n", n_args-1);
        return;
    }

    // cd
    if (!chdir(args[1])) {
        char cwd[150];
        getcwd(cwd, sizeof(cwd));
    } else {
        printf("ERROR: NOT A DIRECTORY: %s\n", args[1]);
    }
} 

void alias(char** args, int n_args) { 
    if(n_args == 2){
        printf("ERROR: EXPECTED 0 OR 2 ARGUMENTS, GOT 1\n");
        return;
    } else if (n_args > 3){
        printf("ERROR: EXPECTED 0 OR 2 ARGUMENTS, GOT %d\n", n_args-1);
        return;
    } else if (n_args == 3){
    
        if (strcmp(args[1], args[2]) == 0){
            printf("ERROR, EXPANSION OF \"%s\" WOULD CREATE A LOOP\n", args[1]);
            return;
        }

        char* key = args[2]; 
        while (key) {
            char* next = NULL;
            for (int i = 0; i < MAX_ALIAS; i++) {
                if (alias_table.occupied[i] == 1 && strcmp(key, alias_table.keys[i]) == 0) {
                    if (strcmp(alias_table.vals[i], args[1]) == 0) {
                        printf("ERROR, EXPANSION OF \"%s\" WOULD CREATE A LOOP\n", args[1]);
                        return;
                    } else {
                        next = alias_table.vals[i];
                        break;
                    }
                }
            }
            key = next;
        }

        // check if currently in the table 
        for (int i = 0; i < MAX_ALIAS; i++) {
            if(alias_table.occupied[i] == 1 && strcmp(args[1], alias_table.keys[i]) == 0) {
                alias_table.vals[i] = args[2];
                return;
            }
        }

        // if not in table, put into the first unoccupied space 
        for (int i = 0; i < MAX_ALIAS; i++) {
            if (alias_table.occupied[i] == 0) {
                alias_table.keys[i] = args[1];
                alias_table.vals[i] = args[2];
                alias_table.occupied[i] = 1;
                return;
            }
        }

        printf("ERROR: MAXIMUM ALIASES REACHED\n");
    // TODO: Make this like printenv
    } else{
        for (int i = 0; i < MAX_ALIAS; i++){
            if (alias_table.occupied[i] == 1){
                printf("%s=%s\n", alias_table.keys[i], alias_table.vals[i]);
            }
        } 
    }
}

void unalias(char** args, int n_args) {
    if (n_args == 0){
        printf("ERROR: EXPECTED 1 ARGUMENT, GOT 0\n");
        return;
    }
    else if (n_args > 2){
        printf("ERROR: EXPECTED 1 ARGUMENT, GOT %d\n", n_args-1);
        return;
    }

    for (int i = 0; i < MAX_ALIAS; i++){
        if (strcmp(alias_table.keys[i], args[1]) == 0){
            alias_table.occupied[i] = 0;
            break;
        }
    } 
}
