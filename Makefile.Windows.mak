CINC	= -Iinclude
CONF	= config
INC		= include
SRC		= src
BIN		= bin

CC       = ml $(CINC)
LEX		= win_flex -i -I
YACC	= win_bison -v
DEL		= del
MOV		= move /y

all: $(SRC)/c1.tab.cpp $(SRC)/c1.lex.cpp

$(SRC)/c1.tab.cpp : $(CONF)/c1.y
	$(YACC) -o $(SRC)/c1.tab.cpp $(CONF)/c1.y


$(SRC)/c1.lex.cpp : $(CONF)/c1.lex
	$(LEX) -o$(SRC)/c1.lex.cpp $(CONF)/c1.lex

clean:
	$(DEL) $(SRC)\c1.lex.cpp
	$(DEL) $(SRC)\c1.tab.cpp
	$(DEL) $(SRC)\c1.output
	$(DEL) $(SRC)\c1.tab.hpp
	$(DEL) $(SRC)\position.hh
	$(DEL) $(SRC)\location.hh
	$(DEL) $(SRC)\stack.hh
