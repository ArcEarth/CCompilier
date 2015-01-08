CINC	 = -Iinclude
CONF	 = config
SRC	 = src
BIN	 = bin
SRCLIST  = $(SRC)/stdafx.cpp $(SRC)/interpreter.cpp $(SRC)/ast_node.cpp $(SRC)/error.cpp $(SRC)/declaration.cpp $(SRC)/decl_context.cpp $(SRC)/driver.cpp $(SRC)/pcode.cpp $(SRC)/type.cpp 

CC       = g++ -std=c++11 $(CINC)
LEX      = flex -i -I 
YACC     = bison
DEL      = rm -f 

all: cc

$(SRC)/c1.tab.cpp : $(CONF)/c1.y
	$(YACC) -o $(SRC)/c1.tab.cpp $(CONF)/c1.y

$(SRC)/c1.lex.cpp : $(CONF)/c1.lex
	$(LEX) -o $(SRC)/c1.lex.cpp $(CONF)/c1.lex

# Generates a compiler for an assignment statement sequence language,
# which constructs an abstract syntax tree for an input program.
cc : $(SRC)/c1.lex.cpp $(SRC)/c1.tab.cpp
	$(CC) -DDEBUG -o $(BIN)/c1mid $(SRC)/c1.lex.cpp $(SRC)/c1.tab.cpp  $(SRCLIST)

clean:
	$(DEL) $(BIN)/cc
	$(DEL) $(SRC)/*.tab.hpp $(SRC)/*.tab.cpp
	$(DEL) test/*.o
	$(DEL) $(SRC)\c1.lex.cpp
	$(DEL) $(SRC)\c1.tab.cpp
	$(DEL) $(SRC)\c1.output
	$(DEL) $(SRC)\c1.tab.hpp
	$(DEL) $(SRC)\position.hh
	$(DEL) $(SRC)\location.hh
	$(DEL) $(SRC)\stack.hh
