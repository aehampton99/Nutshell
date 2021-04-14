Alec Arreche & Anna Hampton
COP4600
Helal

# Nutshell Project

## Description of Roles:

### Alec:
    Alec handled everything having to do with the parser in the .y file. This included parsing the inputs for pipes, io redirection characters, and combinations of command, word, arguments, etc and checking for cycles in aliases. In addition, he implemented the setenv (`setenvir()`), printenv (`printenv()`), unsetenv (`unsetenvir()`), cd (`cd()`), and bye builtin commands. He also completed the functions `init()` ad `main()` where we initialize the necessary variables and set up the parsing respectively. Alec also checked for aliases and checked for cycles in the aliases.

## Anna:
    Anna implemented the alias functionality which includes listing all aliases and adding aliases in the function (`alias()`). She also dealt with unaliasing aliases (`unalias()`). Anna also implemented the functions for handling external commands (`call_extern()`), handling pipes (`piped()`), handling I/O redirection (`redirection()`), and handling wildcard matching. Anna completed the & functionality. In addition, she completed the majority of the README. 

## Together:
    Together, Anna and Alec, completed the .l file and the call function (`call()`) that handles the calls to all the command We also worked together through debugging and error checking. It is important to keep in mind that we did a lot of partner coding to ensure that everything was working correctly, and we changed errors as they appeared regardless of whose assigment it was.

## Features NOT Implemented
1. Filename Completion

## Features Implemented
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
12. Tilde Expansion
