/* 
 * Common definitions and declarations for compilers 
 * Author: Yu Zhang (yuzhang@ustc.edu.cn)
 */
#ifndef _COMMON_H_
#define _COMMON_H_
#include "util.h"

// operator kinds
// You could add more kinds of error messages into errcfg.h 
enum Operator{
#define opxx(a, b) OP_##a,
#include "op.h"
	OPLAST
#undef opxx
};

typedef struct {
	struct type *base;
	int length;
} *Array;

typedef struct {
	struct type *base;
} *Pointer,*Const;

typedef struct type {
	enum {
	#define tpxx(a, b) TP_##a,
	#include "type.h"
	#undef tpxx
//		TP_CONST,
//		TP_ARRAY,
//		TP_POINTER,
//		TP_FUNCION,
//		TP_STRUCT,
//		TP_LAST
	}	kind;
	bool constant;
	bool staticdata;
	int size;
	union {
		Array array;
		Pointer pointer;
//		Const constant;
	};
} *Type;

typedef 
struct {
	Type type;
	union 
	{
		float Float;
		int Int;
		List Array; 
		bool Bool;
		char Char;
//		enum opc op;
	};
} Value;

//the function for type check & type cast
Type TypeCheck(int op,Type left,Type right);
bool TypeMatch(Type A,Type B);
void TypeSystemInitilize();
void dumpType(Type t);
Type newPointer(Type base);
Type newArray(Type base,int length);
Type tracePointer(Type base);
Type traceArray(Type cap,Value length);
Type newConst(Type base);
Type newStatic(Type base);

#define GETVAL(s) ((s).type==TP_BOOL?(s).Bool:((s).type==TP_INT?(s).Int:(s).Float))
#define GETVALP(s) ((s)->type==TP_BOOL?(s)->Bool:((s)->type==TP_INT?(s)->Int:(s)->Float))


//#define error printf
//#define warning printf

//extern char **opname;

enum object {
    OBJ_type, OBJ_var, OBJ_function
};

//instruction define for the 3-addr code

enum fct {
    lit, opr, lod, sto, cal, isp, jmp, jpc, jpe, lar, sar
};
// functions

typedef struct{
    enum fct f;		// function code
    long l; 		// local/global
    long a; 		// displacement address
} instruction;

typedef union{
	long i;
	float f;
	char c[4];
} memunit;

/*
enum fct {
    lit, opr, lod, sto, cal, Int, jmp, jpc         // functions
};

char mnemonic[8][3+1]; //a 

typedef struct{
    enum fct f;		// function code
    long l; 		// level
    long a; 		// displacement address
} instruction;
*/

// symbolic table
typedef struct symbol {
	char	*name;	// name of the symbol
	bool	isInitial;	// whether it is initialized	
	Type   type;	// type of the symbol
	enum object kind;
	struct astnode	*initexp;	// value of the symbol
	int	addr;	// addr of the symbol
	long 	id;	// using this to liner the hash tab.
	int	level;  // if is a local symbol
	struct astnode	*declaration;	//Pointer to the declaration.
} *Symbol;

typedef struct entry {
	struct symbol sym;
	struct entry *next;
} *Entry;

typedef struct fakentry {
	struct entry *enode;
	struct fakentry *next;
} *Fakentry;

typedef struct table {
	// a hashtable to store symbols
	struct entry *buckets[256];
	Fakentry index;
	int top;
	int base[100]; //a stack to store the 
} *Table;
#define HASHSIZE 256

// Function declarations corresponding to symbolic table
Table 	newTable();
Symbol 	lookup(Table ptab, const char *name);
Symbol 	getSym(Table ptab, const char *name);
Symbol	checkSym(Table ptab,const char *name);
void	InitSym(Symbol sym,struct astnode *initexp);
void 	destroyTable();
void	PopTable(Table ptab);
void	PushTable(Table ptab);

// Error/warning message
// You could add more kinds of error messages into errcfg.h 
enum {
#define errxx(a, b) a,
#include "errcfg.h"
	LASTERR
};

// An error/warning message
typedef struct errmsg {
	bool	isWarn;
	int 	type;
	char 	*msg;
	int	line;
	int	column;
} *Errmsg;

// Error factory
typedef struct errfactory { 
	List	errors;
	List	warnings;
} *ErrFactory;

// Function declarations on error message management
Errmsg	newError	(ErrFactory errfactory, int type, int line, int col);
Errmsg	newWarning	(ErrFactory errfactory, int type, int line, int col);
void	dumpErrmsg	(Errmsg error);
ErrFactory newErrFactory();
void	dumpErrors	(ErrFactory errfactory);
void	dumpWarnings	(ErrFactory ecrrfactory);
void	destroyErrFactory(ErrFactory *errfact);

// abstract syntax tree
// Structure for tracking locations, same as YYLTYPE in y.tab.h
typedef struct location {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} *Loc;

/*
typedef struct {
	struct astnode *adrexp;
} *AdrExp;
*/
typedef struct {
	int 	op;
	//int type;
	Type type;
//	Value 	val;
//	bool	IsConst;
	struct astnode	*kids[2];// kids of the AST node
} *Exp;

typedef struct {
	int addr;
	struct astnode *exp;
} *ExpStmt;

typedef struct {
	struct astnode *condition;
	struct astnode *action;
} *WhileStmt;

typedef struct {
	struct astnode *initaction;
	struct astnode *condition;
	struct astnode *action;
	struct astnode *loopaction;
} *ForStmt;

typedef struct {
	struct astnode *condition;
	struct astnode *thenaction;
	struct astnode *elseaction;
} *IfStmt;

typedef struct {
	List  stmts;
} *Block;

typedef struct {
	Type  type;
	List  vars;
} *VarList;

typedef struct {
	Symbol sym;
	struct astnode	*exec;
	struct astnode	*plist;
	int paramsize;
} *Function;

typedef struct {
	Symbol sym;
	struct astnode	*arglist;
} *FunctionCall;

typedef struct {
	struct astnode	*decls;
	struct astnode	*mainfunction;
} *Program;

typedef struct astnode{
	enum {
		KValue = 0x200,
		KName,		//201
		KPrefixExp,	//202
		KInfixExp,	//203
		KAssignExp,	//204
		KParenExp,	//205
		KExpStmt,	//206
		KBlock,		//207
		KDeclBlock,	//208
		KVarDecls,	//209
		KVar,		//20a
		KTypedef,	//20b
		KRef,		//20c
		KFunCall,	//20d
		KAdrExp,	//20e
		KFunctionDecl,	//20f
		KWhileStmt,	//210
		KIfStmt,	//211
		KProgram,	//212
		KMulStmts,	//213
		KParamList,	//214
		KArgList,	//215
		KReturnStmt,	//216
		KBreakStmt,	//217
		KDeRef,		//218
		KIndex,		//219
		KForStmt,	//220
	} kind;	// kind of the AST node
	union {
//		int  val;
		Value val;
		Symbol sym;
		Exp   exp;
//		AdrExp	adrexp;
		ExpStmt  estmt;
		Block  block;
		VarList varlist;
		IfStmt  ifstmt;
		WhileStmt  whstmt;
		ForStmt forstmt;
		Function funcl;
		FunctionCall funcall;
		Program prog;
	};
	Loc 	loc;	// locations
} *ASTNode;

typedef struct ASTtree {
	ASTNode root;
} *ASTTree;

Type getType(ASTNode node);

// functions for creating various kinds of ASTnodes
ASTNode newCast(ASTNode Src,Type target);
ASTNode newNumber(Value value);
ASTNode newPrefixExp(int op, ASTNode exp);
ASTNode newParenExp(ASTNode exp);
ASTNode newInfixExp(int op, ASTNode left, ASTNode right);
ASTNode newAssignment(int op, ASTNode left, ASTNode right);
ASTNode newProgram(ASTNode Decls,ASTNode MainFunc);
ASTNode newBreakStmt();
ASTNode newReturnStmt(ASTNode exp,int ReturnAddr,Type returntype);
ASTNode newDeRef(ASTNode pointer);
ASTNode newIndex(ASTNode array,ASTNode index);
ASTNode newAdrExp(ASTNode ad);
void	destroyProgram(Program *node);
ASTNode newWhileStmt(ASTNode Con,ASTNode Act);
void	destroyWhileStmt(WhileStmt *node);
ASTNode newForStmt(ASTNode InitAct,ASTNode Con,ASTNode LoopAct,ASTNode Act);
void	destroyForStmt(ForStmt *node);
ASTNode newIfStmt(ASTNode Con,ASTNode ThenAct, ASTNode ElseAct);
void	destroyIfStmt(IfStmt *node);
ASTNode newFunCall(Table ptab, char *name, ASTNode Args);
ASTNode newFunc(Table ptab, ASTNode funcnode, ASTNode Params, ASTNode Execute);
void	destroyFunc(Function *pnode);
ASTNode newRef(Table ptab, char *name);
ASTNode newAdrExp(ASTNode adrexp);
ASTNode newVarList(Type type);
//ASTNode newConst(Table ptab, char *name, Type type, Value val);
ASTNode newSym(Table ptab, char *name, Type type);
ASTNode newVar(Table ptab, char *name, Type type);
void	destroyExp(Exp *pexp);
ASTNode newExpStmt(ASTNode exp);
void	destroyExpStmt(ExpStmt *pexpstmt);
ASTNode newBlock();
void	destroyBlock(Block *pblock);
ASTTree newAST();
void	destroyAST(ASTNode *pnode);
void 	dumpAST(ASTNode node);
void	GenAST(ASTNode node);
void 	gen(enum fct x, long y, long z);
Loc	setLoc(ASTNode node, Loc loc);

#endif // !def(_COMMON_H_)
