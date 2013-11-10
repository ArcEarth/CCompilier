CINC	= -Iinclude
CONF	= config
SRC		= src
BIN		= bin
CC		= cl.exe $(CINC)
LEX		= flex.exe -i -I 
YACC	= bison.exe -d -y

$(SRC)\C1.tab.c : $(CONF)\C1.y
	$(YACC) -b -v -o $(SRC)\C1.tab.c $(CONF)\C1.y

$(SRC)\C1.lex.c : $(CONF)\C1.lex
	$(LEX) -o$(SRC)\C1.lex.c $(CONF)\C1.lex

clean : 
	del $(SRC)\C1.tab.c
	del $(SRC)\C1.tab.h
	del $(SRC)\C1.tab.output