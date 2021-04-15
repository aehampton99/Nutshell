nutshell: nutshell.tab.c lex.yy.c main.c
	gcc main.c nutshell.tab.c lex.yy.c -lfl -o nutshell

nutshell.tab.c:
	bison -d nutshell.y

lex.yy.c:
	flex nutshell.l

clean: 
	rm nutshell.tab.c
	rm nutshell.tab.h
	rm nutshell
	rm lex.yy.c
