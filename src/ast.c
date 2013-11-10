/*
 * Functions of Abstract Syntax Tree
 * Author: Yu Zhang (yuzhang@ustc.edu.cn),Yupeng Zhang (ypzhange@mail.ustc.edu.cn)
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

char *opname[]={
#define opxx(a, b) b,
#include "op.h"
	"Undefined Op"
#undef opxx
};

char *typename[]={
#define tpxx(a, b) b,
#include "type.h"
	"Undefined Type"
#undef tpxx
};

extern Type Int,Float,Char,Bool,Void,ConstBool,ConstInt,ConstFloat,ConstChar,String,ConstString; //The Global Var to pointer to the base types.
#define max(a,b) a>b?a:b

ASTNode
newNumber(Value value)
{
	ASTNode new;
	NEW0(new);
	new->kind = KValue;
	new->val = value;
	return new;
}

/*ASTNode
newDeRef(ASTNode pointer)
{
	ASTNode new;
	NEW0(new);
	new->kind = KDeRef;
}

ASTNode
newIndex(ASTNode array,ASTNode index)
{
	ASTNode new;
	NEW0(new);
	new->kind = KDeRef;
}*/

ASTNode
newSym(Table ptab, char *name, Type type)
{
	ASTNode new;
	NEW0(new);
//	new->kind = KVar;
	new->sym = checkSym(ptab,name);
	if (new->sym==NULL){
		new->sym = getSym(ptab, name);
		new->sym->type=type;
//		new->sym->kind=OBJ_var;
		new->sym->declaration = new;
		new->sym->isInitial = 0;
	} else {
		printf("Error : Re-defination of identifier '%s'.\n",name);
	}
//	new->sym = getSym(ptab, name, type);
//	printf("A new Var '%s' in type ",name,new->sym->id);
//	dumpType(type);
//	printf(" has regeisted.\n");
	return new;
}


ASTNode
newVar(Table ptab, char *name, Type type)
{
	ASTNode new;
	NEW0(new);
	new->kind = KVar;
	new->sym = checkSym(ptab,name);
	if (new->sym==NULL){
		new->sym = getSym(ptab, name);
		new->sym->type=type;
		new->sym->kind=OBJ_var;
		new->sym->declaration = new;
		new->sym->isInitial = 0;
	} else {
		printf("Error : Re-defination of identifier '%s'.\n",name);
	}
//	new->sym = getSym(ptab, name, type);
//	printf("A new Var '%s' in type ",name,new->sym->id);
//	dumpType(type);
//	printf(" has regeisted.\n");
	return new;
}
/*
ASTNode
newConst(Table ptab, char *name, Type type, Value val)
{
	ASTNode new;
	NEW0(new);
	new->kind = KConst;
//	new->sym->type=type;
	new->sym = checkSym(ptab,name);
	if (new->sym==NULL){
		new->sym = getSym(ptab, name);
		new->sym->type=type;
	} else {
		printf("Error : Re-defination of identifier '%s'.\n",name);
	}
	if (!TypeMatch(val.type,type))
		{
			printf("Error : Type unmatch in const ""%s"" 's initelize.\n",name);
		}
	new->sym->val = val;//this is a error temp
	new->sym->kind = constant;
	new->sym->isInitial = 1;
	new->sym->declaration = new;
	printf("A new Const '%s' was regeistedin id=%ld\n",name,new->sym->id);
//	new->sym = getSym(ptab, name, type);
	return new;
}*/
/*
ASTNode
newFunc(Table ptab, char *name, Type type, ASTNode Params, ASTNode Execute)
{
	ASTNode new;
	Function newf;
	NEW0(new);
	NEW0(newf);
	newf->sym = lookup(ptab,name);
	newf->sym->type = type;
	newf->sym->isInitial = 0;
	newf->plist=Params;
	newf->exec=Execute;
	newf->sym->declaration = new;
	newf->sym->kind = OBJ_function;
	new->kind = KFunctionDecl;
	new->funcl = newf;
	return new;
}*/// OldVersion

ASTNode
newFunc(Table ptab, ASTNode funcnode, ASTNode Params, ASTNode Execute)
{
	Function newf;
	NEW0(newf);
	newf->sym = funcnode->sym;
	newf->plist=Params;
	newf->exec=Execute;
	funcnode->funcl = newf;
	return funcnode;
}


void
destroyFunc(Function *pnode)
{
	Function node=*pnode;
	destroyAST(&node->exec);
	free(node);
	pnode=NULL;
}

ASTNode
newRef(Table ptab, char *name)
{
	ASTNode new;
	NEW0(new);
	new->kind = KRef;
	new->sym = lookup(ptab, name);
	if (new->sym==NULL) printf("Error : Identifire '%s' not regeisted!\n",name);
	return new;
}

void InitSym(Symbol sym,ASTNode initexp)
{
//	debug("Initlizing...\n");
	if (initexp==NULL)
		sym->isInitial = 0;
	else {
//	debug("S1\n");
		if (!TypeMatch(getType(initexp),sym->type))
			{
				printf("Error : Type unmatch in var ""%s"" 's initelize.\n",sym->name);
			}
//		debug("S2\n");
		sym->isInitial = 1;
		sym->initexp = initexp;
	}
//	debug("Initlized.\n");
}

/*
ASTNode
newAdrExp(ASTNode adrexp)
{
//	return aexp;
	printf("AdrExpStart!");
	ASTNode new;
	NEW0(new);
	new->kind = KAdrRef;
	Exp newexp;
	NEW0(newexp);
	newexp->op = OP_REF;
	newexp->kids[0] = aexp;
//	newexp->type = TypeCheck(op,getType(exp),Void);
	new->exp = newexp;
	printf("AdrExpFinish!");
	return new;
}
*/

ASTNode
newFunCall(Table ptab, char *name, ASTNode Args)
{
	ASTNode new;
	NEW0(new);
	FunctionCall newfc;
	NEW0(newfc);
	new->kind = KFunCall;
	newfc->sym = lookup(ptab, name);
	newfc->arglist = Args;
	new->funcall = newfc;
	//check if the arg list matches with param list 
//	printf("NewFunCallS1\n");
/*	List Plist = newfc->sym->declaration->funcl->plist->block->stmts;
	List Alist = Args->block->stmts;
	ListItr Pitr = newListItr(Plist, 0);
	ListItr Aitr = newListItr(Alist, 0);
	ASTNode param,arg;
//	printf("NewFunCallS2\n");
	while ( hasNext(Pitr)&&hasNext(Aitr) )  {
		param = (ASTNode)nextItem(Pitr);
		arg = (ASTNode)nextItem(Aitr);
//		printf("NewFunCallS2.5\n");
		TypeCheck(OP_ASGN,getType(param),getType(arg));
	}
//	printf("NewFunCallS3\n");
	if (hasNext(Pitr)&&!hasNext(Aitr))
	{
		printf("Error : There is too few arguments for function %s.\n",name);
	} else if (!hasNext(Pitr)&&hasNext(Aitr)) {
		printf("Error : There is too many arguments for function %s.\n",name);
	}
	free(Pitr);
	free(Aitr);*/
//	printf("NewFunCallF\n");
	return new;
}

ASTNode
newPrefixExp(int op, ASTNode exp)
{
	ASTNode new;
	NEW0(new);
	new->kind = KPrefixExp;
	Exp newexp;
	NEW0(newexp);
	newexp->op = op;
	newexp->kids[0] = exp;
	newexp->type = TypeCheck(op,getType(exp),Void);
	new->exp = newexp;
	return new;
}

ASTNode
newParenExp(ASTNode exp)
{
	ASTNode new;
	NEW0(new);
	new->kind = KParenExp;
	Exp newexp;
	NEW0(newexp);
	newexp->op = -1;
	newexp->kids[0] = exp;
	newexp->type = getType(exp);
	new->exp = newexp;
	return new;
}

ASTNode
newInfixExp(int op, ASTNode left, ASTNode right)
{
	ASTNode new;
	NEW0(new);
	new->kind = KInfixExp;
	Exp newexp;
	NEW0(newexp);
	newexp->op = op;
	newexp->type = TypeCheck(op,getType(left),getType(right));
	newexp->kids[0] = left;
	newexp->kids[1] = right;
	new->exp = newexp;
	return new;
}

ASTNode
newCast(ASTNode Src,Type target)
{
	if ((getType(Src)->kind==TP_FLOAT)&&(target->kind<TP_FLOAT)||(getType(Src)->kind<TP_FLOAT)&&(target->kind==TP_FLOAT)){
	ASTNode new;
	NEW0(new);
	new->kind = KPrefixExp;
	Exp newexp;
	NEW0(newexp);
	newexp->op = OP_CAST;
	newexp->kids[0] = Src;
	newexp->type = target;
	new->exp = newexp;
	return new;
	}
	else return Src;
}

ASTNode
newAssignment(int op, ASTNode left, ASTNode right)
{
//	printf("Assignment Start.\n");
	ASTNode new;
	NEW0(new);
	new->kind = KAssignExp;
//	printf("Assignment 1.\n");
//	if (left->sym->kind == constant) 
//	  printf("Error : Const var ""%s"" can not be assign.\n",left->sym->name);
//	printf("Assignment 2.\n");
	Exp newexp;
	NEW0(newexp);
	newexp->op = op;
	newexp->type=TypeCheck(op,getType(left),getType(right));
/*	if ((getType(left)->kind==TP_INT)&&(getType(right)->kind==TP_FLOAT)||(getType(left)->kind==TP_FLOAT)&&(getType(right)->kind==TP_INT))
		right = newPrefixExp(OP_CAST,right);*/
	right = newCast(right,newexp->type);
	newexp->kids[0] = left;
	newexp->kids[1] = right;
	new->exp = newexp;
//	printf("Assignment Finish.\n");
	return new;
}

void
destroyExp(Exp *pnode)
{
	Exp node = *pnode;
	destroyAST(&node->kids[0]);
	destroyAST(&node->kids[1]);
	free(node);
	*pnode = NULL;
}

ASTNode
newExpStmt(ASTNode exp)
{
	ASTNode new;
	NEW0(new);
	new->kind = KExpStmt;
	ExpStmt newstmt;
	NEW0(newstmt);
	new->estmt = newstmt;
	newstmt->exp = exp;
	return new;
}

ASTNode
newBreakStmt()
{
	ASTNode new;
	NEW0(new);
	new->kind = KBreakStmt;
	return new;
}

ASTNode
newReturnStmt(ASTNode exp,int ReturnAddr,Type returntype)
{
	ASTNode new;
	NEW0(new);
	new->kind = KReturnStmt;
	ExpStmt newstmt;
	NEW0(newstmt);
	ReturnAddr -= returntype->size;
	new->estmt = newstmt;
	if (returntype==Void) {
		newstmt->exp = NULL;
		return new;
	}
	TypeCheck(OP_ASGN,returntype,getType(exp));
	newstmt->exp = newCast(exp,returntype);
	newstmt->addr = ReturnAddr;
	return new;
}

void
destroyExpStmt(ExpStmt *pnode)
{
	ExpStmt node = *pnode;
	destroyAST(&node->exp);
	free(node);
	*pnode = NULL;
}

ASTNode
newBlock()
{
	ASTNode new;
	NEW0(new);
	new->kind = KBlock;
	Block newb;
	NEW0(newb);
	new->block = newb;
	newb->stmts = newList();
	return new;
}

ASTNode
newVarList(Type type)
{
	ASTNode new;
	NEW0(new);
//	new->kind = KBlock;
	VarList newv;
	NEW0(newv);
	new->varlist = newv;
	newv->vars = newList();
	newv->type = type;
	return new;
}

ASTNode
newProgram(ASTNode Decls,ASTNode MainFunc)
{
	ASTNode new;
	NEW0(new);
	new->kind = KProgram;
	Program newp;
	NEW0(newp);
	new->prog = newp;
	newp->decls = Decls;
	newp->mainfunction = MainFunc;
	return new;
}

void
destroyProgram(Program *pnode)
{
	Program node=*pnode;
	destroyAST(&node->decls);
	destroyAST(&node->mainfunction);
	free(node);
	*pnode=NULL;
}

ASTNode
newIfStmt(ASTNode Con,ASTNode ThenAct, ASTNode ElseAct)
{
	ASTNode new;
	NEW0(new);
	new->kind = KIfStmt;
	IfStmt newi;
	NEW0(newi);
	new->ifstmt = newi;
	newi->condition = Con;
	newi->thenaction = ThenAct;
	newi->elseaction = ElseAct;
	return new;
}

void
destroyIfStmt(IfStmt *pnode)
{
//	printf("DestroyingIfStmt\n");
	IfStmt node=*pnode;
//	ASTNode *Con=&(node->condition);
//	printf("ConAdress : %x\n",Con);
//	printf("DestroyingIfStmtStep2\n");
	destroyAST(&node->condition);
//	printf("DestroyingifStmtStep2.5\n");
	destroyAST(&node->thenaction);
//	printf("DestroyingIfStmtStep3\n");
	free(node);
	*pnode=NULL;
}

ASTNode
newWhileStmt(ASTNode Con,ASTNode Act)
{
	ASTNode new;
	NEW0(new);
	new->kind = KWhileStmt;
	WhileStmt neww;
	NEW0(neww);
	new->whstmt = neww;
	neww->condition = Con;
	neww->action = Act;
	return new;
}

ASTNode
newForStmt(ASTNode InitAct,ASTNode Con,ASTNode LoopAct,ASTNode Act){
	ASTNode new;
	NEW0(new);
	new->kind = KForStmt;
	ForStmt newf;
	NEW0(newf);
	new->forstmt = newf;
	newf->condition = Con;
	newf->action = Act;
	newf->initaction = InitAct;
	newf->loopaction = LoopAct;
	return new;
}

void
destroyWhileStmt(WhileStmt *pnode)
{
//	printf("DestroyingWhileStmt\n");
	WhileStmt node=*pnode;
	destroyAST(&node->condition);
	destroyAST(&node->action);
	free(node);
	*pnode=NULL;
}

void
destroyForStmt(ForStmt *pnode)
{
//	printf("DestroyingWhileStmt\n");
	ForStmt node=*pnode;
	destroyAST(&node->condition);
	destroyAST(&node->action);
	destroyAST(&node->loopaction);
	destroyAST(&node->initaction);
	free(node);
	*pnode=NULL;
}

void
destroyBlock(Block *pnode)
{
	Block node = *pnode;
	destroyList(&node->stmts);
	free(node);
	*pnode = NULL;
}

ASTTree
newAST()
{
	ASTTree new;
	NEW0(new);
	return new;
}

void
destroyAST(ASTNode *pnode)
{
//	printf("DestryingAST\n");
//	printf("ASTNodeAdress : %x\n",pnode);
	ASTNode node = *pnode;
//	printf("DsstryingAST1.1\n");
	int kind = node->kind;
//	printf("DestryingAST,Kind=%d\n",kind);
	switch (kind) {
	case KValue:
	case KName:
	case KRef:
	case KFunCall:
		break;
	case KFunctionDecl:
		destroyFunc(&node->funcl);
		break;
	case KPrefixExp:
	case KParenExp:
	case KInfixExp:
	case KAssignExp:
		destroyExp(&node->exp);
		break;
	case KExpStmt:
		destroyExpStmt(&node->estmt);
		break;
	case KVarDecls:
//	case KConstDecls:
	case KMulStmts:
	case KDeclBlock:
	case KBlock:
		destroyBlock(&node->block);
		break;
	case KProgram:
		destroyProgram(&node->prog);
		break;
	case KIfStmt:
		destroyIfStmt(&node->ifstmt);
		break;
	case KWhileStmt:
		destroyWhileStmt(&node->whstmt);
		break;
	case KForStmt:
		destroyForStmt(&node->forstmt);
		break;
	default:
		printf("Unhandled ASTNode kind!\n");
	}
	free(node);
	*pnode = NULL;
}

Loc
setLoc(ASTNode node, Loc loc)//fline, int fcol, int lline, int lcol)
{
	if (node->loc == NULL )
		NEW0(node->loc);
	node->loc->first_line = loc->first_line;
	node->loc->first_column = loc->first_column;
	node->loc->last_line = loc->last_line;
	node->loc->last_column = loc->last_column;
	return node->loc;
}

void
dumpAST(ASTNode node)
{
//	debug("\nstep0\n");	
	int kind = node->kind;
#ifdef DEBUG
	printf("\nnode type : %x\n",kind);
#endif
	switch (kind) {
	case KValue:
	{
		PrintVal(&node->val);
		break;
	}
	case KVar:
	{
//		printf("%d\n",(int)(node->sym));
//		printf("var detected!\n");
		printf("%s", node->sym->name);
		if (node->sym->isInitial)
		{
			printf("=");
			dumpAST(node->sym->initexp);
		}
		break;
	}
	case KTypedef:
	{
		printf("typedef ");
		dumpType(node->sym->type);
		printf(" %s;",node->sym->name);
		break;
	}	
	case KRef:
	{
//		debug("\nstepName1\n");
//		printf("%x\n",(int)(node->sym));
//		printf("%d\n",(int)(node->));
//		printf("(");
//		dumpType(getType(node));
//		printf(")");
		printf("%s", node->sym->name);
//		printf("\nstepName2\n");
		break;
	}
/*	case KConst:
	{
		printf("%s=", node->sym->name);
		PrintVal(&node->sym->val);
		break;
	}*/
	case KFunCall:
	{
//		char *sda=node->sym->name;
//		int dfsh=node->sym->id;
//		printf("\nFunction Call Start.\n");
//		printf("[");
//		dumpType(getType(node));
//		printf("]");
		printf("%s(", node->funcall->sym->name);
		dumpAST(node->funcall->arglist);
		printf(")(id=%2ld)", node->funcall->sym->id);
//		printf("\nFunction Call Finish.\n");
		break;
	}
	case KReturnStmt:
	{
		printf("return ");
		dumpAST(node->estmt->exp);
		printf(";");
		break;
	}
	case KBreakStmt:
	{
		printf("break;");
		break;
	}
	case KAdrExp:
	case KDeRef:
	case KPrefixExp:
		printf("%s(", opname[node->exp->op]);
		dumpAST(node->exp->kids[0]);
		printf(")");
		break;
	case KParenExp:
	{
		printf("(");
		dumpAST(node->exp->kids[0]);
		printf(")");
		break;
	}
	case KIndex:
	{
//		printf("[type=%s]",typename[getType(node)]);
		dumpAST(node->exp->kids[0]);
		printf("[");
		dumpAST(node->exp->kids[1]);
		printf("]");
		break;
	}
	case KInfixExp:
	{
//		printf("[type=%s]",typename[getType(node)]);
		dumpAST(node->exp->kids[0]);
		printf("%s", opname[node->exp->op]);
		dumpAST(node->exp->kids[1]);
		break;
	}
	case KAssignExp:
	{
//		debug("\nstepAssign1\n");
		dumpAST(node->exp->kids[0]);
//		debug("\nstepAssign2\n");
//		printf("op is %d\n",node->exp->op);
		printf("%s", opname[node->exp->op]);
//		debug("\nstepAssign3\n");
		dumpAST(node->exp->kids[1]);
		break;
	}
	case KExpStmt:
	{
		printf("(");
		dumpType(getType(node->estmt->exp));
		printf(")");
		dumpAST(node->estmt->exp);
		printf(";");
		break;
	}
	case KFunctionDecl:
	{
		dumpType(node->funcl->sym->type);
		printf(" %s(",node->funcl->sym->name);
		if (node->funcl->plist)
			dumpAST(node->funcl->plist);
		printf(")\n");
		dumpAST(node->funcl->exec);
		break;
	}
	case KParamList:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
//		printf("%s ",typename[((ASTNode)getFirst(node->block->stmts))->sym->type]);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
		ASTNode param;
		while ( hasNext(itr) )  {
			param = (ASTNode)nextItem(itr);
			dumpType(param->sym->type);
			printf(" ");
//			printf("%s ",typename[->kind]);
			dumpAST(param);
			//nextItem(itr);
			printf(",");
		}
//		printf("\b");
		free(itr);
		break;
	}
	case KArgList:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
//		printf("%s ",typename[((ASTNode)getFirst(node->block->stmts))->sym->type]);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
//		ASTNode param;
		while ( hasNext(itr) )  {
//			param = (ASTNode)nextItem(itr);
//			printf("%s ",typename[param->sym->type]);
			dumpAST((ASTNode)nextItem(itr));
			//nextItem(itr);
			printf(",");
		}
		printf("\b");
		free(itr);
		break;
	}
	case KVarDecls:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
//		printf("%s ",typename[node->varlist->type->kind]);
		dumpType(node->varlist->type);
		printf(" ");
		List vars = node->varlist->vars;
		ListItr itr = newListItr(vars, 0);
		while ( hasNext(itr) )  {
			dumpAST((ASTNode)nextItem(itr));
			//nextItem(itr);
			printf(",");
		}
		printf("\b;");
		free(itr);
		break;
	}
/*	case KConstDecls:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		printf("const %s ",typename[((ASTNode)getFirst(node->block->stmts))->sym->type->kind]);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
		while ( hasNext(itr) )  {
			dumpAST((ASTNode)nextItem(itr));
			//nextItem(itr);
			printf(",");
		}
		printf("\b;");
		free(itr);
		break;
	}*/
	case KDeclBlock:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
//		debug("stepKDB1\n");		
		while ( hasNext(itr) )  {
			dumpAST((ASTNode)nextItem(itr));
			printf("\n");
		}
//		debug("stepKDB2\n");
		
		free(itr);
		break;
	}
	case KBlock:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
		while ( hasNext(itr) )  {
			dumpAST((ASTNode)nextItem(itr));
			printf("\n");
		}
		free(itr);
		break;
	}
	case KMulStmts:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		printf("{\n");
		List stmts = node->block->stmts;
//		debug("step1\n");
		ListItr itr = newListItr(stmts, 0);
//		debug("step2\n");
		while ( hasNext(itr) )  {
//			debug("step3\n");
			dumpAST((ASTNode)nextItem(itr));
			printf("\n");
		}
		free(itr);
		printf("}");
		break;
	}
	case KProgram:
	{
		dumpAST(node->prog->decls);
		dumpAST(node->prog->mainfunction);
		printf("\n");
		printf("AST Dump Finish.\n");
		break;
	}
	case KWhileStmt:
	{
		printf("while (");
		dumpAST(node->whstmt->condition);
		printf(")\n");
		dumpAST(node->whstmt->action);
		break;
	}
	case KForStmt:
	{
		printf("for (");
		dumpAST(node->forstmt->initaction);
		dumpAST(node->forstmt->condition);
		printf(";");
		dumpAST(node->forstmt->loopaction);
		printf(")\n");
		dumpAST(node->forstmt->action);
		break;
	}
	case KIfStmt:
	{
		printf("if (");
		dumpAST(node->ifstmt->condition);
		printf(")\n");
		dumpAST(node->ifstmt->thenaction);
		if (node->ifstmt->elseaction)
		{
			printf("\nelse\n");
			dumpAST(node->ifstmt->elseaction);
		}
		break;
	}
	default:
	{
		printf("Unhandled ASTNode kind : %x!\n",node->kind);
		break;
	}
	}
//	printf("\nexit %x\n",kind);
}


