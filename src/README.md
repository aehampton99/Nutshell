Alec Arreche & Anna Hampton
COP4600
Helal

# Nutshell Project

## Description of Roles:

### Alec:
    Alec handled everything having to do with the parser in the .y file, which largely consisted of creating parser support for functions in `main.c`. 
    This included parsing the inputs for pipes, io redirection characters, and combinations of command, word, arguments, etc. In addition, he implemented the setenv (`setenvir()`), printenv (`printenv()`), unsetenv (`unsetenvir()`), cd (`cd()`), and bye builtin commands. 
    He also completed the functions `init()` ad `main()` where we initialize the necessary variables and set up the parsing respectively. 
    Alec also checked for aliases, checked for cycles in the aliases, and handled the environment variable expansion and alias expansion.

### Anna:
    Anna implemented the alias functionality which includes listing all aliases and adding aliases in the function (`alias()`). 
    She also dealt with unaliasing aliases (`unalias()`). 
    Anna also implemented the functions for handling external commands (`call_extern()`), handling pipes (`piped()`), handling I/O redirection (`redirection()`), and handling wildcard matching (`list_files()` and `handle_wild()` located in the .y). Anna completed the & functionality. 
    In addition, she completed the majority of the README. 

### Together:
    Together, Anna and Alec, completed the .l file and the call function (`call()`) that handles the calls to all the commands. We also worked together through debugging and error checking. 
    It is important to keep in mind that we did a lot of partner coding to ensure that everything was working correctly, and we changed errors as they appeared regardless of whose assigment it was.

## Features NOT Implemented:
1. Tilde Expansion
2. Filename Completion

## Features Implemented:
1. Setting environment (`setenvir()`)
2. Printing environment (`printenv()`)
3. Unsetting environment (`unsetenvir()`)
4. Environment variables PATH and HOME
5. Changing directory (`cd()`)
6. Adding aliases (`alias()`)
7. Listing aliases (`alias()`)
8. Unsetting aliases (`unalias()`)
9. Exiting shell
10. Environment variable expansion
11. Wildcard matching

## Things to Note:
- Default value for `${HOME}` is the current working directory. 
- Defualt value for `${PATH}` is `.:/usr/bin:/bin`
- This shell does **not** support spaces in file paths. 
- The shell supports up to the following limits: 
    - Maximum number of environment variables: `100`
    - Maximum number of aliases: `100`
    - Maximum character length of file path: `300`
    - Maximum number of wildcard matched files: `100`
    - Maximum number of command line arguments: `100`
    - Maximum number of pipes: `100`
- This shell supports excess whitespaces in between command arguments and at the end of the line, but NOT at the beginning. 
- When executing a command in background using `&` that outputs to the console, the prompt for a command `>` may be offset in following commands. 
