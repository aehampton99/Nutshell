Alec Arreche & Anna Hampton
COP4600
Helal

# Nutshell Project

## Description of Roles:
Alec:
- Setenv (`setenvir()`)
- Printenv (`printenv()`)
- Unsetenv (`unsetenvir()`)
- CD
- Bye
- .y file/parser stuff
- Main function (`main()`)
- Wildcard Matching
- Checking for aliases
- Checking for cycles in aliases

Anna:
- Alias function (`alias()`)
  - adding aliases
  - listing aliases
- Unalias function (`unalias()`)
- Handling external commands (`call_extern()`)
- Handling pipes (`piped()`)
- Handling io redirection (`redirection()`)

Together:
- .l file
- Call function (`call()`)
- Initializer function for environment variables and alias (`init()`)
- Worked together through debugging and error checking
  - Important to keep in mind that we did a lot of partner coding to ensure that everything was working correctly and we changed errors as they appeared regardless of whose assigment it was.

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
