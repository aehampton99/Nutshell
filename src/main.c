#include "init.h"
#include "nutshell.tab.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h> 
#include <fnmatch.h>

int yylex();
extern char* yytext;
extern int yyparse();
int BYE = 0;

void init();
void setenvir(char** args, int n_args);
//void unsetenvir(char** args, int n_args);
void printenv(char** args, int n_args);
void cd(char** args, int n_args);
void alias(char** args, int n_args);
void unalias(char** args, int n_args);
void call_extern(char** args, int n_args);
void piped(char*** cmd, int* n_cmd_args, int n_cmds);
void redirection();
char** list_files();
int filecmp(char* s, char* t);

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
    var_table.vals[1] = ".:/usr/bin:/bin"; // needs legit default value 
    PATH = var_table.vals[1];
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

    int ampersand = 1;
    char** filenames = list_files;

    if (strcmp(cmd, "setenv") == 0) {
        setenvir(args, n_args);
    } else if (strcmp(cmd, "printenv") == 0) {
        printenv(args, n_args);
    } //else if (strcmp(cmd, "unsetenvir") == 0) {
    //     unsetenvir(args, n_args);} 
    else if (strcmp(cmd, "cd") == 0) {
        cd(args, n_args);
    } else if (strcmp(cmd, "alias") == 0) {
        alias(args, n_args);
    } else if (strcmp(cmd, "unalias") == 0) {
        unalias(args, n_args);
    } else if (strcmp(cmd, "bye") == 0) {
        BYE = 1;
    } else if (strcmp(cmd, "hey") == 0){
        list_files();
    }else {
        call_extern(args, n_args); 
    }

    return 0;
}

int filecmp(char* s, char* t){
    return strcmp(s,t);
}

char* str_slice(char* str, int start, int end){
    int length = end - start;
    char* out[length];

    for (int i = start; i <= end; i++) {
        out[i] = str[i];
    }

    out[++length] = 0;

    return out;
}

//char** list_files(char* pattern, int patternType){
char** list_files(){
    const char* pattern = "t*.txt";
    char* filenames[MAX_FILES];

    DIR* dir;
    struct dirent *fl;
    char cwd[300];
    getcwd(cwd, sizeof(cwd));
    printf("Executing in %s: \n", cwd);

    if ((dir = opendir(cwd)) != NULL) {
        int flnum = 0;
        while ((fl = readdir(dir)) != NULL) {
            char* fname = fl->d_name;

            if(fnmatch(pattern, fname, 0 ) == 0 ){
                filenames[flnum++] = fname;
            }
        }
        filenames[flnum] = NULL;

        closedir (dir);

        if (flnum > 1){
            qsort(filenames, flnum, sizeof(filenames[0]), filecmp);
        }

        for (int i = 0; i < flnum; i++){
            printf("File %d in matching files: %s\n", i, filenames[i]);
        }

        return filenames;
    } else {
        printf("Could not open directory.");
        return EXIT_FAILURE;
    }
}


void call_extern(char** args, int n_args) {
    // for each in path 
    char* path_copy[strlen(var_table.vals[1])];
    strcpy(path_copy, var_table.vals[1]);
    char* token;

    token = strtok(path_copy, ":");

    char* result[INT16_MAX];
    int ampersand = 0;

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
        if (ampersand == 0){
            wait(0);
        }
    }
}

void piped(char*** cmds, int* n_cmd_args, int n_cmds){
    // char *ls[] = {"cat","test.txt", NULL};
    // char *grep[] = {"sort", NULL};
    // //char *wc[] = {"grep", "-v", "'200[456]'", NULL};
    // //char *more[] = {"more", NULL};
    // char **cmd[] = {ls, grep, NULL};

    // int n_cmd_args[4] = {2, 1};
    // int n_cmds = 2;

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

//void redirection(char** args, int n_args, int piping, char*** cmds, int n_cmd_args, int n_cmds){
void redirection(){
    // when pipes redirection arguments, in this case "args", would begin with the first redirection metacharacter
    // when no pipes, it includes the command and everything else
    // the three possible patterns
    // command with io redirection -> redirection()
    // piped commands with io redirection -> redirection()
    // piped commands -> piped()
    // everything needs to be NULL terminated
    char *args[] = {">", "f5.txt", NULL};
    int n_args = 2;

    char *ls[] = {"cat","test.txt", NULL};
    char *grep[] = {"sort", NULL};
    char **cmd[] = {ls, grep, NULL};
    int n_cmd_args[4] = {2, 1};
    int n_cmds = 2;

    int piping = 1;

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
        for (int i = 0; i < n_args; i++){
            int j;
            if (!strcmp(args[i], "<")){
                ++i;
                input = open(args[i], O_RDONLY);

                dup2(input, STDIN_FILENO);
                close(input);
                continue;
            }
            if (!strcmp(args[i], ">")){
                ++i;
                output = creat(args[i], 0644);

                dup2(output, STDOUT_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], ">>")){
                ++i;
                append = open(args[i], O_CREAT | O_RDWR | O_APPEND, 0644);

                dup2(append, STDOUT_FILENO);
                close(append);
                continue;
            }
            if (!strcmp(args[i], "2>")){
                ++i;
                output = creat(args[i], 0644);

                dup2(output, STDERR_FILENO);
                close(output);
                continue;
            }
            if (!strcmp(args[i], "2>&1")){
                ++i;
                dup2(STDOUT_FILENO, STDERR_FILENO);
                close(stdout);
                continue;
            }
            cleaned[c++] = args[i];
        }
        cleaned[c] = NULL;
        if (piping == 1){
            piped(cmd, n_cmd_args, n_cmds);
        }
        else{
            call(cleaned, c);
        }
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
        for (int i = 0; i < MAX_ALIAS; i++){
            if (alias_table.occupied[i] == 0){
                continue;
            }
            printf("%s = %s\n", alias_table.keys[i], alias_table.vals[i]);
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
