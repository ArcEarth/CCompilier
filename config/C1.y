/*
 * asgn.y : A simple yacc assignment statement parser
 *          Based on the Bison manual example. 
 * The parser computes each assignment and output its value.
 * Author: Yu Zhang (yuzhang@ustc.edu.cn)
 */

%{
#include <stdio.h>
#include <math.h>
#include <common.h>
extern Table symtab;
extern ASTTree ast;
extern Type Int,Float,Char,Bool,Void,ConstBool,ConstInt,ConstFloat,ConstChar,String; //
static Type TypeCache;
static Type ReturnType;
static int paramcounter,ReturnAddr;
static Symbol FunctionCallCache;
static ListItr Pitr;

int yyerror(char *message);
extern int yylex();
%}

%union {
//	float fval;
	char *name;
	int  ival;
	Value val;
	Type tval;
	ASTNode node;
	Symbol sym;
}
%locations

%token NUMBER UNREGID TYPEID FUNCTIONID VARID BASETYPE
%token CONSTSYM TYPEDEFSYM STATICSYM
%token MAINSYM
%token WHILESYM FORSYM IFSYM ELSESYM
%token BREAKSYM RETURNSYM
%token COMMA SEMICOLON
%token BEGINSYM ENDSYM
%token READSYM WRITESYM
%token PLUS MINUS MULT DIV ASGN GEQ GTR LSS LEQ EQL NEQ ANDAND OROR NOT AND OR XOR LSH RSH BITNOT SIZEOF
%token EOL
%token LPAREN RPAREN
%token LBRACK RBRACK

%left  USELESS
%left  NOELSE
%left  ELSESYM
%right ASGN
%left  OROR
%left  ANDAND
%left  OR
%left  XOR
%left  AND
%left  EQL NEQ
%left  GEQ GTR LSS LEQ
%left  LSH RSH
%left  MINUS PLUS
%left  MULT DIV MOD 
%left REFENCE
%right NEG NOT ADDRESS DEREFENCE BITNOT SIZEOF
%left  INDEX GROUP LBRACK LPAREN

%type  <ival> PLUS MINUS MULT DIV MOD ASGN GEQ GTR LSS LEQ EQL NEQ ANDAND OROR NOT AND OR XOR LSH RSH BITNOT SIZEOF
%type  <tval> Type BASETYPE
%type  <val>  NUMBER ArrayValue
%type  <name> UNREGID TYPEID VARID FUNCTIONID
%type  <node> AdrExp Exp Statement DeclRegon DeclStatement VarDecl  FuncDecl TypeDecl MulStatement Program Main ParamList ArgList DelicateID InitExp MAINSYM
%%
Program	: DeclRegon Main
	  {
	    	$$ = newProgram($1 , $2);
		setLoc($$, (Loc)&(@$));
	    	ast->root =$$;
	    	debug("Program->DeclRegon Main EOF\n");
	  }
	;

DeclRegon: /*Empty*/
	  {
	    	$$ = newBlock();
		setLoc($$, (Loc)&(@$));
		$$->kind = KDeclBlock;
	  }
	| DeclRegon DeclStatement
	  {
//	    	debug("A Decled has been noticed!\n");
	    	$$ = $1;
//		printf("Add to List %x\n",(int)($$->block->stmts));
	    	addLast($$->block->stmts , $2);
	  }
	;

DeclStatement: VarDecl SEMICOLON
	  {
	    	$$ = $1;
		setLoc($$, (Loc)&(@$));
	    	debug("VarDecled ;\n");
	  }
	| FuncDecl
	  {
	    	$$ = $1;
		setLoc($$, (Loc)&(@$));
	    	debug("FunctionDecled\n");
	  }
	| TypeDecl
	  {
		$$ = $1;
		setLoc($$, (Loc)&(@$));
	  }
	;

Type	: BASETYPE
	  {
		$$ = $1;
		TypeCache = $$;
//		printf("%d\n",$$->kind);
	  }
	| TYPEID //History Err : This rule will cause some fatal shift-reduce conflicts like code stmt : function (a,b); will be reduce into Type (ID,ID) and cause syntax errors.
	  {
		$$=lookup(symtab,$1)->type;
		TypeCache = $$;
	  }
	| CONSTSYM Type
	  {
		$$ = newConst($2);
		TypeCache = $$;
//		printf("%d\n",$$->kind);
	  }
	| STATICSYM Type
	  {
		$$ = $2;
		$$ = newStatic($2);
		TypeCache = $$;
	  }
	;

VarDecl : Type DelicateID InitExp
	  {
		$2->sym->kind = OBJ_var;
		$2->kind = KVar;

		$$ = newVarList($1);
	    	$$->kind = KVarDecls;
		InitSym($2->sym,$3);
	    	addLast($$->varlist->vars,$2);
		printf("A new Var '%s' in type ",$2->sym->name);
		dumpType($2->sym->type);
		printf(" has regeisted.\n");
		setLoc($$, (Loc)&(@$));
	  }
	| Type error
	  {
	    	printf("Error : VarDecl syntax error %4d(%2d)\n", @$.first_line, @$.last_column);		
	  }
	| VarDecl COMMA DelicateID InitExp
	  {
	    	$$ = $1;
		$3->sym->kind = OBJ_var;
		$3->kind = KVar;
		InitSym($3->sym,$4);
	    	addLast($$->varlist->vars,$3);
		printf("A new Var '%s' in type ",$3->sym->name);
		dumpType($3->sym->type);
		printf(" has regeisted.\n");
		setLoc($$, (Loc)&(@$));
	  }
	;

DelicateID : UNREGID
	  {
		$$ = newSym(symtab,$1,TypeCache);
	  }
	| VARID
	  {
		$$ = newSym(symtab,$1,TypeCache);
	  }
	| FUNCTIONID
	  {
		$$ = newSym(symtab,$1,TypeCache);
	  }
	| MULT DelicateID %prec DEREFENCE
	  {
		$$ = $2;
		$$->sym->type = tracePointer($$->sym->type);
	  }
	| DelicateID LBRACK NUMBER RBRACK %prec INDEX
	  {
		$$ = $1;
		$$->sym->type = traceArray($$->sym->type,$3);
	  }
	| LPAREN DelicateID RPAREN
	  {
		$$ = $2;
	  }
	;
/*
ComplexValue : NUMBER
	  {
		$$ = $1;
	  }
	| ArrayValue ENDSYM
	  {
		$$ = $1;
	  }
	;
*/
ArrayValue : BEGINSYM NUMBER
	  {
		Value *v;
		NEW(v);
		*v = $2;
		$$.type = newArray($2.type,1);
		$$.Array = newList();
		addLast($$.Array,v);
	  }
	|
	ArrayValue COMMA NUMBER
	  {
		Value *v;
		NEW(v);
		*v = $3;
		$$ = $1;
		$$.type->array->length++;
		addLast($$.Array,v);
	  }
	;

InitExp : 
	  {
		$$ = NULL;
	  }
	| ASGN Exp
	  {
		$$ = $2;
		setLoc($$, (Loc)&(@$));
	  }
	| ASGN ArrayValue ENDSYM
	  {
		$$ = newNumber($2);
		setLoc($$, (Loc)&(@$));
	  }
	;

TypeDecl : TYPEDEFSYM Type DelicateID SEMICOLON
	  {
		$$ = $3;
		$$->sym->kind = OBJ_type;
		$$->kind = KTypedef;
		printf("A new Type '%s' is Declared in %4d(%2d)\n",$$->sym->name, @$.first_line, @$.last_column);
		setLoc($$, (Loc)&(@$));
	  }
	| TYPEDEFSYM error SEMICOLON
	  {
		printf("Error : Type Declare statement error @%4d(%2d)\n", @$.first_line, @$.last_column);
		setLoc($$, (Loc)&(@$));
	  }
	;

/*FuncDecl: Type UNREGID LPAREN 
	  {
		if(checkSym(symtab, $2)==NULL) 
			getSym(symtab, $2);
		else 	printf("Error : Re-defination of identifier '%s'.\n",$2);
		PushTable(symtab);
		paramcounter=0;
		ReturnType = $1;
//Important,the id of func must be decl in global symtab
	  }
	  ParamList RPAREN Statement
	  {
		PopTable(symtab);
		$$ = newFunc(symtab,$2,$1,$5,$7);
		$$->funcl->paramsize = paramcounter;
		paramcounter=0;
		setLoc($$, (Loc)&(@$));
		ReturnType = Void;
	  }
	;*///old Version

FuncDecl: Type DelicateID LPAREN 
	  {
		$2->sym->kind = OBJ_function;
		$2->kind = KFunctionDecl;
		printf("A new function %s has %d kind\n",lookup(symtab,$2->sym->name)->name,lookup(symtab,$2->sym->name)->kind);
/*		if(checkSym(symtab, $2)==NULL) 
			getSym(symtab, $2);
		else 	printf("Error : Re-defination of identifier '%s'.\n",$2);*/
		PushTable(symtab);
		paramcounter=0;
		ReturnType = $1;
//Important,the id of func must be decl in global symtab
	  }
	  ParamList 
	  {
		$2 = newFunc(symtab,$2,$5,NULL);
	  }
	  RPAREN Statement
	  {
		PopTable(symtab);
//		$$ = newFunc(symtab,$2,$5,$7);
		$$ = $2;
		$$->funcl->exec = $8;
		$$->funcl->paramsize = paramcounter;
//		printf("%s Kind : %d\n",$$->funcl->sym->name,lookup(symtab,"Breath")->kind);
		paramcounter=0;
		setLoc($$, (Loc)&(@$));
		ReturnType = Void;
	  }
	;


ParamList:
	  {
//		$$ = NULL;
	    	$$ = newBlock();
	    	$$->kind = KParamList;
		setLoc($$, (Loc)&(@$));
//		printf("Add to List %x\n",(int)($$->block->stmts));
//	    	addLast($$->block->stmts,newVar(symtab,$2,$1));
	  }
	| Type DelicateID
	  {
	    	$$ = newBlock();
	    	$$->kind = KParamList;
		setLoc($$, (Loc)&(@$));
		addLast($$->block->stmts,$2);
		paramcounter-=$2->sym->type->size;
		$2->kind = KVar;
		$2->sym->kind = OBJ_var;
		$2->sym->addr = paramcounter;
//		addLast($$->block->stmts,newVar(symtab,$2,$1));
	  }
	| ParamList COMMA Type DelicateID
	  {
	    	$$ = $1;
		if (!$$) {
			printf("Error : Expect for Type bufore ','.\n");
	    		$$ = newBlock();
	    		$$->kind = KParamList;
			setLoc($$, (Loc)&(@$));
		}
		addLast($$->block->stmts,$4);
		paramcounter-=$4->sym->type->size;
		$4->kind = KVar;
		$4->sym->kind = OBJ_var;
		$4->sym->addr=paramcounter;
//	    	addLast($$->block->stmts,newVar(symtab,$4,$3));
	  }
	;

Main	: BASETYPE MAINSYM LPAREN RPAREN
	  {
		$2 = newSym(symtab,"main",$1);
		$2->sym->kind = OBJ_function;
		$2->kind = KFunctionDecl;
		PushTable(symtab);
		paramcounter=0;
		ReturnType = $1;
	  }
	  Statement
 	  {
		PopTable(symtab);
		$$ = newFunc(symtab,$2,NULL,$6);
		$$->funcl->paramsize = paramcounter;
		paramcounter=0;
		ReturnType = Void;
		setLoc($$, (Loc)&(@$));
	  }
	;

Statement: SEMICOLON
	  {
		printf("Empty Statement!\n");
		$$ = NULL;
	  }
	| Exp SEMICOLON
	  {
		$$ = newExpStmt($1);
		setLoc($$, (Loc)&(@$));
	  }
	| FORSYM
	  {
		PushTable(symtab);
	  }
	  LPAREN Statement Exp SEMICOLON Exp RPAREN Statement
	  {
		PopTable(symtab);
		if ($4->kind!=KExpStmt&&$4->kind!=KVar)
		{
			printf("Error : For statement could not contain a statement in the location of initelize expression.\n");
		}
		$$ = newForStmt($4,$5,newExpStmt($7),$9);
		setLoc($$, (Loc)&(@$));
	  }
	| WHILESYM LPAREN Exp RPAREN Statement
	  {
		$$ = newWhileStmt($3,$5);
		setLoc($$, (Loc)&(@$));
	  }
	| BREAKSYM SEMICOLON
	  {
		$$ = newBreakStmt();
		setLoc($$, (Loc)&(@$));
	  }
	| RETURNSYM SEMICOLON
	  {
		$$ = newReturnStmt(NULL,ReturnAddr,Void);
		setLoc($$, (Loc)&(@$));
	  }
	| RETURNSYM Exp SEMICOLON 
	  {
		$$ = newReturnStmt($2,paramcounter,ReturnType);
		setLoc($$, (Loc)&(@$));
	  }
	| IFSYM LPAREN Exp RPAREN Statement ELSESYM Statement %prec ELSESYM
	  {
		$$ = newIfStmt($3,$5,$7);
		setLoc($$, (Loc)&(@$));
	  }
	| IFSYM LPAREN Exp RPAREN Statement %prec NOELSE
	  {
		$$ = newIfStmt($3,$5,NULL);
		setLoc($$, (Loc)&(@$));
	  }
	| BEGINSYM
	  {
		PushTable(symtab);
	  }
	  MulStatement ENDSYM
	  {
		PopTable(symtab);
		$$ = $3;
		setLoc($$, (Loc)&(@$));
	  }
	| VarDecl SEMICOLON
	  {
		$$ = $1;
		setLoc($$, (Loc)&(@$));
	  }
	| READSYM LPAREN Exp RPAREN SEMICOLON
	  {
		$$ = newPrefixExp(OP_READ,$3);
	    	setLoc($$, (Loc)&(@$));
	  }
	| WRITESYM LPAREN Exp RPAREN SEMICOLON
	  {
		$$ = newPrefixExp(OP_WRITE,$3);
	    	setLoc($$, (Loc)&(@$));
	  }
	| error
	  {
	    	printf("Stmt Error @ : %4d(%2d)\n", @$.first_line, @$.last_column);		
	  }
	;

MulStatement:
	  {
	    	$$ = newBlock();
	    	$$->kind = KMulStmts;
		setLoc($$, (Loc)&(@$));
	  }
	| MulStatement Statement
	  {
		$$ = $1;
		addLast($$->block->stmts,$2);
	    	debug("Statement ;\n");
	  }
	;

ArgList :
	  {
		printf("ArgList => /* Nothing */!\n");
//	    	$$ = NULL;
	    	$$ = newBlock();
	    	$$->kind = KArgList;
		Pitr = newListItr(FunctionCallCache->declaration->funcl->plist->block->stmts, 0);
	  }
	| Exp
	  {
		debug("ArgList => Exp\n");
//		printf("FunctionCallCache : %x\n",FunctionCallCache->declaration->kind);
		Pitr = newListItr(FunctionCallCache->declaration->funcl->plist->block->stmts, 0);
//		debug("ArgList step2!\n");
	    	$$ = newBlock();
	    	$$->kind = KArgList;

		ASTNode param;
		if (!hasNext(Pitr))
		{
			printf("Error : There is too many arguments for function %s.\n",FunctionCallCache->name);
			debug("ArgList => Exp Error\n");
		}
		else
		{
			param = (ASTNode)nextItem(Pitr);
			$1 = newCast($1,getType(param));
	/*		if (TypeCheck(OP_ASGN,getType(param),getType($1))->kind!=getType($1)->kind)
				$1 = newPrefixExp(OP_CAST,$1);*/
			addFirst($$->block->stmts,$1);
			setLoc($$, (Loc)&(@$));
			debug("ArgList => Exp Finish\n");
		}
	  }
	|
	ArgList COMMA Exp
	  {
		debug("ArgList =>ArgList , Exp\n");
		$$ = $1;
		if (!$$) {
			printf("Error : Expect for Type bufore ','.\n");
	    		$$ = newBlock();
	    		$$->kind = KArgList;
		}

		ASTNode param;
		if (hasNext(Pitr))
		{ 
			param = (ASTNode)nextItem(Pitr);
			$3 = newCast($3,getType(param));
			addFirst($$->block->stmts,$3);
		    	debug("ArgList ,\n");		
		}
		else	printf("Error : There is too many arguments for function %s.\n",FunctionCallCache->name);
	  }
	;

Exp     : NUMBER
	  {
	    	debug("exp => NUMBER\n");
	    	$$ = newNumber($1);
	    	setLoc($$, (Loc)&(@$));
	  }
	| AdrExp// %prec REFENCE
	  {	// %prec REFENCE
	    	debug("exp => AdrExp\n");
	    	$$ = newPrefixExp(OP_REF, $1); 
		$$->kind = KAdrExp;
	    	setLoc($$, (Loc)&(@$));
	  }
	| AND AdrExp %prec ADDRESS
	  {
	    debug("exp => & AdrExp\n");
	    $$ = newPrefixExp(OP_ADDRESS, $2); 
	    setLoc($$, (Loc)&(@$));
	  } 
	| FUNCTIONID 
	  {
		debug("Exp => Function (Start)\n");
		FunctionCallCache = lookup(symtab,$1);
	  }
	  LPAREN ArgList RPAREN
	  {
		debug("exp => ID (ArgList)\n");
//		$$ = newName(symtab, $1);
//	    	$$ = newRef(symtab, $1);
		if (hasNext(Pitr)) printf("Error : There is too few arguments for function %s.\n",$1);
		$$ = newFunCall(symtab, $1, $4);
//	    	debug("CallFunction\n");
		setLoc($$, (Loc)&(@$));
	  }
        | BITNOT Exp
	  {
	    debug("exp => ~ exp\n");
	    $$ = newPrefixExp($1, $2); 
	    setLoc($$, (Loc)&(@$));
	  }
        | NOT Exp
	  {
	    debug("exp => ! exp\n");
	    $$ = newPrefixExp($1, $2); 
	    setLoc($$, (Loc)&(@$));
	  }
	| SIZEOF Exp
 	  {
	    debug("exp => sizeof exp\n");
	    $$ = newPrefixExp($1, $2); 
	    setLoc($$, (Loc)&(@$));
	  }
       | MINUS Exp %prec NEG
	  {
	    debug("exp => - exp\n");
	    $$ = newPrefixExp(OP_NEG, $2); 
	    setLoc($$, (Loc)&(@$));
	  }
	| AdrExp ASGN Exp %prec ASGN
	  {
	    	debug("AsgnExp => AExp ASGN exp\n");
	    	$$ = newAssignment($2, $1, $3);
	    	setLoc($$, (Loc)&(@$));
	  }
 	| Exp EQL Exp
	  {
	    	debug("exp => exp == exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp NEQ Exp
	  {
	    	debug("exp => exp != exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp LEQ Exp
	  {
	    	debug("exp => exp <= exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp GTR Exp
	  {
	    	debug("exp => exp > exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp LSS Exp
	  {
	    	debug("exp => exp < exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp GEQ Exp
	  {
	    	debug("exp => exp >= exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp PLUS Exp
	  {
	    	debug("exp => exp + exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp MINUS Exp
	  {
	    	debug("exp => exp - exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp MULT Exp
	  {
	    	debug("exp => exp * exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp DIV Exp
	  {
	    	debug("exp => exp / exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp MOD Exp
	  {
	    	debug("exp => exp % exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp ANDAND Exp
	  {
	    	debug("exp => exp && exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp OROR Exp
	  {
	    	debug("exp => exp || exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp AND Exp
	  {
	    	debug("exp => exp & exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp OR Exp
	  {
	    	debug("exp => exp | exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp XOR Exp
	  {
	    	debug("exp => exp ^ exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp LSH Exp
	  {
	    	debug("exp => exp << exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp RSH Exp
	  {
	    	debug("exp => exp >> exp\n");
	    	$$ = newInfixExp($2, $1, $3); 
	    	setLoc($$, (Loc)&(@$));
	  }
        | LPAREN Exp RPAREN %prec GROUP
	  {
	    debug("exp => ( exp )\n");
	    $$ = newParenExp($2);
	    setLoc($$, (Loc)&(@$));
	  }
        ;

AdrExp:	VARID
	  {
	    	debug("AExp => VARID\n");
	    	$$ = newRef(symtab, $1);
//		$$ = newName(symtab, $1);
	    	setLoc($$, (Loc)&(@$));
	  }
	| UNREGID
	  {
	    	debug("AExp => UNREGID\n");
	    	$$ = newRef(symtab, $1);
//		$$ = newName(symtab, $1);
	    	setLoc($$, (Loc)&(@$));
	  }
	| MULT AdrExp %prec DEREFENCE
	  {
		//Use this produce equltion to provide shift-reduce conflicts like *p=3 to reduce into *(p=3);
	    	debug("AdrExp => * AdrExp\n");
	    	$$ = newPrefixExp(OP_DEREF, $2); 
		$$->kind = KDeRef;
	    	setLoc($$, (Loc)&(@$));
	  }
	| MULT Exp %prec DEREFENCE
	  {
	    	debug("AdrExp => * Exp\n");
	    	$$ = newPrefixExp(OP_DEREF, $2); 
		$$->kind = KDeRef;
	    	setLoc($$, (Loc)&(@$));
	  }
	| Exp LBRACK Exp RBRACK %prec INDEX
	  {
	    	debug("AdrExp => Exp [ Exp ]\n");
	    	$$ = newInfixExp(OP_INDEX, $1, $3); 
		$$->kind = KIndex;
	    	setLoc($$, (Loc)&(@$));
	  }	
	| LPAREN AdrExp RPAREN
	  {
		$$ = $2;
	  }
	;


%%

int yyerror(char *message)
{
	printf("%s\n",message);
	return 0;
}
