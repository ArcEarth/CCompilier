#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

static int dx,cx,lev;
static int LoopBreakPoint=0;
#define cxmax 2000
static instruction code[cxmax+1];

extern memunit StaticMemery[];
extern int smp;
extern FILE *outfile; //the esir code output file handler

static char mnemonic[][4]={"lit","opr","lod","sto","cal","isp","jmp","jpc","jpe","lar","sar"};

void gen(enum fct x, long y, long z){
    	if(cx>cxmax){
		printf("program too long\n");
		exit(1);
    	}
	if ((x==lar)&&(code[cx-1].f==lit)&&(!y)&&(!z))
	{
//		printf("oping!\n");
		code[cx-1].f = lod;
		return;
	}	//A optizime
	if ((x==sar)&&(code[cx-1].f==lit)&&(!y)&&(!z))
	{
//		printf("oping!\n");
		code[cx-1].f = sto;
		return;
	}	//A optizime
	if ((x==isp)&&(!y)&&(!z))
	{
		return;
	}
/*	if ((x==jmp)&&(code[cx-1].f==lit)&&(!y)&&(!z)&&(code[z].f==jpc))
	{
		code[cx-1].f = jmp;
		code[cx-1].l = code[z].l;
		code[cx-1].a = code[z].a;
		return;
	}*/	//Break optizime
    	code[cx].f=x; code[cx].l=y; code[cx].a=z;
    	cx=cx+1;
}

void 
GenValue(Value *val)
{
	if (val->type->kind==TP_ARRAY)
	{
		ListItr Itr=newListItr(val->Array,0);
		while (hasNext(Itr)){
			GenValue((Value*)nextItem(Itr));
		}
		return;
	}
	if (val->type->staticdata)
	{
		gen(lit,2,val->Int);
		return;
	}
	gen(lit,0,val->Int);
}

void
GenAST(ASTNode node)
{
//	debug("\nstep0\n");	
	int kind = node->kind;
#ifdef DEBUG
//	printf("\nnode type : %x @ %d\n",kind,cx);
#endif
	switch (kind) {
	case KValue:
	{
/*		if (node->val.type->kind==TP_ARRAY)
		{
			ListItr Itr=newListItr(node->val.Array,0);
			while (hasNext(Itr)){
				gen(lit,0,((Value*)nextItem(Itr))->Int);
			}
			break;
		}
		if (node->val.type->staticdata)
//		if (TypeMatch(node->val.type,String))
		{
			gen(lit,2,node->val.Int);
			break;
		}
		gen(lit,0,node->val.Int);*/
		GenValue(&(node->val));
		break;
	}
	case KTypedef: break;
	case KAdrExp:
		GenAST(node->exp->kids[0]);
		ASTNode adr = node->exp->kids[0];
		if (getType(adr)->kind!=TP_ARRAY)
			gen(lar,0,0);
//		dumpType(getType(adr));
//		printf(" in %d\n",cx);
		break;
	case KRef:
	{
		if (!node->sym->type->staticdata)
		gen(lit,node->sym->level>0,node->sym->addr);
		else	gen(lit,2,node->sym->addr);
//		printf("Ref %s address [%d]\n",node->sym->name,node->sym->addr);
		break;
/*		switch (node->sym->type->kind)
		{
		case TP_INT:
		case TP_BOOL:
		case TP_FLOAT:
		case TP_POINTER:
			gen(lod,node->sym->level>0,node->sym->addr);
			break;
		case TP_ARRAY:
			gen(lit,node->sym->level>0,node->sym->addr);
		}
		break;*/
	}
	case KVar:
	{
		if (node->sym->type->staticdata)
		{
			node->sym->addr=smp;	
			StaticMemery[smp].i=0;
			if (node->sym->isInitial) 
			if (node->sym->initexp->kind==KValue)
				StaticMemery[smp].i=node->sym->initexp->val.Int;
			else {
				printf("Error : static var must initelize with a const value");
			}
			smp+=node->sym->type->size;
			node->sym->level=lev;
		} else {
			node->sym->addr=dx;
			dx+=node->sym->type->size;
			node->sym->level=lev;
			if (node->sym->isInitial) 
				GenAST(node->sym->initexp);
			else	gen(isp,0,node->sym->type->size);
		}
//		printf("%s", node->sym->name);
//		printf("%d\n",(int)(node->sym));
//		printf("there is a new var @%d!\n",cx);
//		dumpType(node->sym->type);
//		printf(" %s is regeisted in address = %d\n",node->sym->name,node->sym->addr);
		break;
	}
/*	case KConst:
	{
		node->sym->addr=dx++;
		node->sym->level=lev;
		gen(lit,0,node->sym->val);
//printf("%s=%d", node->sym->name, node->sym->val);
		break;
	}*/
	case KFunCall:
	{
		gen(isp,0,getType(node)->size);//place a space to store return value
		GenAST(node->funcall->arglist);
		gen(cal,0,node->funcall->sym->addr);
		gen(isp,0,node->funcall->sym->declaration->funcl->paramsize);
//		printf("%s()", node->sym->name);
		break;
	}
	case KPrefixExp:
//		printf("%s", opname[node->exp->op]);
		if (node->exp->op==OP_SIZEOF)
		{
			gen(lit,0,getType(node)->size);
			break;
		}
		if (node->exp->op==OP_READ)
		{
			gen(opr,node->exp->type->kind,OP_READ);
			GenAST(node->exp->kids[0]);
			gen(sar,0,0);
			gen(isp,0,-1);
			break;
		}
		GenAST(node->exp->kids[0]);
		if (node->exp->op==OP_CAST)
		{
			gen(opr,getType(node->exp->kids[0])->kind!=TP_FLOAT,OP_CAST);
			break;
		}
		if (node->exp->op==OP_WRITE)
		{
			gen(opr,node->exp->type->kind,OP_WRITE);
			break;
		}
		if (node->exp->op!=OP_ADDRESS)
			gen(opr,0,node->exp->op);
		break;
	case KParenExp:
	{
//		printf("(");
		GenAST(node->exp->kids[0]);
//		printf(")");
		break;
	}
	case KInfixExp:
	{
		int cx1;
		switch (node->exp->op)
		{
		case OP_OROR:
			GenAST(node->exp->kids[0]);
			cx1=cx;
			gen(jpe,1,0);
	//		printf("%s", opname[node->exp->op]);
			GenAST(node->exp->kids[1]);
			gen(opr,0,node->exp->op);//op is the key!
			code[cx1].a=cx;
			break;
		case OP_ANDAND:
			GenAST(node->exp->kids[0]);
			cx1=cx;
			gen(jpc,1,0);
	//		printf("%s", opname[node->exp->op]);
			GenAST(node->exp->kids[1]);
			gen(opr,0,node->exp->op);//op is the key!
			code[cx1].a=cx;
			break;
		default:
			GenAST(node->exp->kids[0]);
	//		printf("%s", opname[node->exp->op]);
			GenAST(node->exp->kids[1]);
			int overload = (getType(node->exp->kids[0])->kind==TP_FLOAT)*2+(getType(node->exp->kids[1])->kind==TP_FLOAT);
			gen(opr,overload,node->exp->op);//op is the key!
			break;
		}
		break;
	}
	case KDeRef:
	{
		GenAST(node->exp->kids[0]);
//		gen(lar,0,0);
		break;
	}
	case KIndex:
	{
		GenAST(node->exp->kids[0]);
		GenAST(node->exp->kids[1]);
		if (node->exp->type->size!=1)
		{
			gen(lit,0,node->exp->type->size);
			gen(opr,0,OP_MULT);
		}
		gen(opr,0,OP_PLUS);
		break;
	}
	case KAssignExp:
	{
//		GenAST(node->exp->kids[0]);
//		printf("%s", opname[node->exp->op]);
//		debug("\nstepAssign3\n");
		GenAST(node->exp->kids[1]);
		ASTNode id=node->exp->kids[0];
/*		if (id->kind==KRef) {
			gen(sto,id->sym->level>0,id->sym->addr);
		}
		else {*/
/*		if ((getType(node->exp->kids[1])->kind==TP_FLOAT)&&(getType(node->exp->kids[0])->kind==TP_INT))
			gen(opr,0,OP_CAST);
		if ((getType(node->exp->kids[1])->kind==TP_INT)&&(getType(node->exp->kids[0])->kind==TP_FLOAT))
			gen(opr,1,OP_CAST);*/
		GenAST(id);
		gen(sar,0,0);
//		}
		break;
	}
	case KExpStmt:
	{
		GenAST(node->estmt->exp);
		gen(isp,0,-getType(node->estmt->exp)->size);	//descard the temp var exp.place
//		printf(";");
		break;
	}
	case KFunctionDecl:
	{
//		printf("%s %s()\n",typename[node->funcl->sym->type],node->funcl->sym->name);
		int cx1=cx;
		gen(jmp,0,0);
		node->funcl->sym->addr=cx;//cx=current ip
		int dx1=dx;
		dx=2;lev++;
		gen(isp,0,2);
		GenAST(node->funcl->exec);
		gen(opr,0,0);
		code[cx1].a=cx;
		dx=dx1;lev--;
		break;
	}
	case KVarDecls:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
//		printf("%s ",typename[((ASTNode)getFirst(node->block->stmts))->sym->type]);
		List vars = node->varlist->vars;
		ListItr itr = newListItr(vars, 0);
		while ( hasNext(itr) )  {
			GenAST((ASTNode)nextItem(itr));
			//nextItem(itr);
//			printf(",");
		}
//		printf("\b;");
		free(itr);
		break;
	}
	case KDeclBlock:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
//		debug("stepKDB1\n");		
		while ( hasNext(itr) )  {
			GenAST((ASTNode)nextItem(itr));
//			printf("\n");
		}
//		debug("stepKDB2\n");
		
		free(itr);
		break;
	}
	case KArgList:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		List args = node->block->stmts;
		ListItr itr = newListItr(args, 0);
		while ( hasNext(itr) )  {
			GenAST((ASTNode)nextItem(itr));
//			printf("\n");
		}
		free(itr);
		break;
	}

	case KBlock:
	{
//		printf("This blcok size = %d\n",node->block->stmts->size);
		List stmts = node->block->stmts;
		ListItr itr = newListItr(stmts, 0);
		while ( hasNext(itr) )  {
			GenAST((ASTNode)nextItem(itr));
//			printf("\n");
		}
		free(itr);
		break;
	}
	case KMulStmts:
	{
		int dx1=dx;
//		lev++;
//		printf("This blcok size = %d\n",node->block->stmts->size);
//		printf("{\n");
		List stmts = node->block->stmts;
//		debug("step1\n");
		ListItr itr = newListItr(stmts, 0);
//		debug("step2\n");
		while ( hasNext(itr) )  {
//			debug("step3\n");
			GenAST((ASTNode)nextItem(itr));
//			printf("\n");
		}
		free(itr);
		if (dx!=dx1)
			gen(isp,0,dx1-dx);
		dx=dx1;
//		lev;
//		printf("}");
		break;
	}
	case KProgram:
	{
		dx=2;cx=0;lev=0;
//		gen(isp,0,-1);
		GenAST(node->prog->decls);
//		code[0].a=cx;
//		gen(isp,0,1);	//the useless return value of main function
		gen(cal,0,cx+2);//
		GenAST(node->prog->mainfunction);
//		gen(jmp,0,0);
		gen(opr,0,0);
		long i;

		printf("Code Generated Start:\n");
		for (i=0;i<cx;i++)
		printf("%10ld%5s%3ld%5ld\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		printf("Code Generated Finish.\n");

		if(outfile==NULL){
			fclose(outfile);
			exit(1);
		}
		printf("StaticMemery[%d] : ",smp);
		for (i=0;i<smp;i++)
		{
			int j;
			for (j=0;j<4;j++)
			putchar(StaticMemery[i].c[j]);
		}
		putchar('\n');
		fwrite(&smp,sizeof(long),1,outfile);
		fwrite(StaticMemery,sizeof(memunit),smp,outfile);
//		char flag=0xFFFF;
//		fwrite(&flag,)
		fwrite(&cx,sizeof(long),1,outfile);
		fwrite(code,sizeof(instruction),cx,outfile);
		fclose(outfile);

		break;	
	}
	case KWhileStmt:
	{
		long cx1,cx2,Lbp1;
//		printf("while (");
		cx1=cx;
		GenAST(node->whstmt->condition);
		cx2=cx;
		Lbp1=LoopBreakPoint;
		LoopBreakPoint=cx;
		gen(jpc,0,0);
//		printf(")\n");
		GenAST(node->whstmt->action);
		LoopBreakPoint=Lbp1;
		gen(jmp,0,cx1);
		code[cx2].a=cx;
		break;
	}
	case KForStmt:
	{
		long cx1,cx2,Lbp1;
//		printf("while (");
		GenAST(node->forstmt->initaction);
		cx1=cx;
		GenAST(node->forstmt->condition);
		cx2=cx;
		Lbp1=LoopBreakPoint;
		LoopBreakPoint=cx;
		gen(jpc,0,0);
//		printf(")\n");
		GenAST(node->forstmt->action);
		GenAST(node->forstmt->loopaction);
		LoopBreakPoint=Lbp1;
		gen(jmp,0,cx1);
		code[cx2].a=cx;
		break;
	}
	case KBreakStmt:
	{
		if (!LoopBreakPoint)
			{
				printf("Error : You can not use a BREAK out side a loop stmt.\n");
			}
		gen(lit,0,0);
		gen(jmp,0,LoopBreakPoint);
		break;
	}
	case KIfStmt:
	{
		long cx1;
//		printf("if (");
		GenAST(node->ifstmt->condition);
		cx1=cx;
		gen(jpc,0,0);
//		printf(")\n");
		GenAST(node->ifstmt->thenaction);
		if (node->ifstmt->elseaction!=NULL)
		{
			gen(jmp,0,0);
			code[cx1].a=cx;
			cx1=cx-1;
			GenAST(node->ifstmt->elseaction);
			code[cx1].a=cx;
		}	code[cx1].a=cx;
		break;
	}
	case KReturnStmt:
	{
		GenAST(node->estmt->exp);
		gen(sto,1,node->estmt->addr);
		gen(opr,0,0);
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
