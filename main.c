#include <stdio.h>
#include "nutshell.tab.h"

extern int yyparse(); 
extern char* yytext;

int main(){

    while(1){
        yyparse();
    }
}