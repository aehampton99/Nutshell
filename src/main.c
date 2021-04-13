#include "init.h"
#include "nutshell.tab.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int yylex();
extern char* yytext;
extern int yyparse();

void init();
void setenvir(char** args, int n_args);
//void unsetenvir(char** args, int n_args);
void printenv(char** args, int n_args);
void cd(char** args, int n_args);
void alias(char** args, int n_args);
void unalias(char** args, int n_args);
void call_extern(char** args, int n_args);
void piped();
void redirection();

int main() {
    init();
    printf("Nutshell\n");

    while(BYE == 0) {
        printf(">");
        yyparse();
        // if (yyparse() == 1)
        //     break;
    }
    printf("...done\n");
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
    var_table.vals[1] = "/usr/bin:/bin"; // needs legit default value 
    PATH = var_table.vals[1];

    // set BYE
    BYE = 0;
}

// main entry point for a command 
int call(char** args, int n_args) {
    //printf("Printing Args in Main.C\n");
    //for (int i = 0; i < n_args; i++) {
    //    printf("%s\n", args[i]);
    //}

    // TODO: check for aliases first 
    // check for built in command 
    char* cmd = args[0];

    if (strcmp(cmd, "setenv") == 0) {
        setenvir(args, n_args);
    } else if (strcmp(cmd, "printenv") == 0) {
        printenv(args, n_args);
    // } else if (strcmp(cmd, "unsetenvir") == 0) {
    //     unsetenvir(args, n_args);
    } else if (strcmp(cmd, "cd") == 0) {
        cd(args, n_args);
    } else if (strcmp(cmd, "alias") == 0) {
        alias(args, n_args);
    } else if (strcmp(cmd, "unalias") == 0) {
        unalias(args, n_args);
    } else if (strcmp(cmd, "bye") == 0) {
        BYE = 1;
    } else if (strcmp(cmd, "hey") == 0){
        redirection();
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
    int worked = 0;

    while (token != NULL) {
        // try to execute
        //printf("executing %s in %s\n", args[0], token);

        strcpy(result, token);
        strcat(result, "/");
        strcat(result, args[0]);

        //printf("executing %s\n", result);

        // fork
        pid_t p;

        p = fork();

        // check fork worked
        if (p < 0){
            printf("Fork failed.");
        } else if (p == 0){
            //printf("Args 1: %s\n", args[0]);
            //printf("Args 2: %s\n", args[1]);
            //printf("Number of args: %d\n", n_args);

            char cwd[150];
            getcwd(cwd, sizeof(cwd));
            //printf("%s\n", cwd);

            if(execv(result, args) != -1){
                worked = 1;
                break;
            }
            else {
                worked = -1;
            }

            exit(&p);
        }
        else {
           wait(0);
        }

        // go next 
        token = strtok(NULL, ":");
    }

    if (worked == -1){
        printf("Execution failed.\n");
    }
}

void piped(){
    char *ls[] = {"cat","test.txt", NULL};
    char *grep[] = {"sort", NULL};
    //char *wc[] = {"grep", "-v", "'200[456]'", NULL};
    //char *more[] = {"more", NULL};
    char **cmd[] = {ls, grep, NULL};

    int n_cmd_args[4] = {2, 1};
    int n_cmds = 2;

    int fd1[2], fd2[2];
    pid_t p;

    int i = 0;
    for (int i = 0; i < n_cmds; i++){
        pipe(fd1);

        p = fork();

        if (p < 0){
            printf("Houston we have a problem.\n");
            exit(1);
            return;
        }
        else if (p == 0){
            dup2(fd2[0], STDIN_FILENO);

            if (cmd[i + 1] != NULL){
                dup2(fd1[1], STDOUT_FILENO);
            }

            close(fd1[0]);
            call(cmd[i], n_cmd_args[i]);
            exit(0);
        }
        else{
            wait(&p);

            close(fd1[1]);
            fd2[0] = fd1[0];
        }
    }
}

void redirection(){
    char *args[] = {"cat", "missing.txt", "2>", "f.txt", NULL};
    int n_args = 3;

    int input = 0, output = 0, append = 0;
    char* cleaned[n_args];
    int c = 0;

    pid_t p;
    p = fork();

    if (p < 0){
        printf("Houston, we have a problem.\n");
        exit(1);
        return;
    } else if (p == 0){
        for (int i = 0; i < n_args-1; i++){
            int j = i+1;

            if (!strcmp(args[i], "<")){
                input = open(args[j], O_RDONLY);

                dup2(input, STDIN_FILENO);
                close(input);
                continue;
            }
            if (!strcmp(args[i], ">")){
                output = creat(args[j], 0644);

                dup2(output, STDOUT_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], ">>")){
                append = open(args[j], O_CREAT | O_RDWR | O_APPEND, 0644);

                dup2(append, STDOUT_FILENO);
                close(append);
                continue;
            }
            if (!strcmp(args[i], "2>")){
                output = creat(args[j], 0644);

                dup2(output, STDERR_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], "2>&1")){
                dup2(STDOUT_FILENO, STDERR_FILENO);
                close(STDOUT_FILENO);
                continue;
            }
            cleaned[c++] = args[i];
        }
        cleaned[c] = NULL;
        call(cleaned, c);
        exit(0);
    } else {
        wait(&p);
    }
}

void setenvir(char** args, int n_args) {
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
        return;
    }

    // cd
    if (!chdir(args[1])) {
        char cwd[150];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    } else {
        printf("ERROR: NOT A DIRECTORY: %s\n", args[1]);
    }

    // TODO: wildcard matching 
}

void alias(char** args, int n_args) { 
    if(n_args == 2){
        printf("ERROR: EXPECTED 0 OR 2 ARGUMENTS, GOT 1\n");
        return;
    } else if (n_args > 3){
        printf("ERROR: EXPECTED 0 OR 2 ARGUMENTS, GOT %d\n", n_args-1);
        return;
    }

    if (n_args == 3){
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
                printf("entered at index %d\n", i);
                return;
            }
        }

        printf("ERROR: MAXIMUM ALIASES REACHED\n");
    } else{
        printf("Args 1: %s\n", args[0]);
        printf("Args 2: %s\n", args[1]);
        for (int i = 0; i < MAX_ALIAS; i++){
            if (alias_table.occupied[i] == 0){
                continue;
            }
            printf("%s\n", alias_table.vals[i]);
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
        if (alias_table.keys[i] == args[2]){
            alias_table.keys[i] = " ";
            alias_table.vals[i] = " ";
            alias_table.occupied[i] = 0;
        }
    } 
}
