CINC	= -Iinclude
CONF	= config
SRC		= src
BIN		= bin
LEX		= flex -i -I 
YACC	= bison -d -y
DEL		= del

$(SRC)\C1.tab.c : $(CONF)\C1.y
	$(YACC) -b -v -o $(SRC)\C1.tab.c $(CONF)\C1.y

$(SRC)\C1.lex.c : $(CONF)\C1.lex
	$(LEX) -o$(SRC)\C1.lex.c $(CONF)\C1.lex

clean : 
	$(DEL) $(SRC)\C1.tab.c
	$(DEL) $(SRC)\C1.tab.h
	$(DEL) $(SRC)\C1.lex.c