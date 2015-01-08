#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "pcodegen.h"


PcodeGenerater newPcodeGenerater()
{
	PcodeGenerater this;
	NEW0(this);
	this->code = (instruction*) calloc((cxmax), sizeof(instruction));
	return this;
}

void destroyPcodeGenerater(PcodeGenerater *pThis)
{
	PcodeGenerater this = *pThis;
	if (this == NULL) return;
	free(this->code);
	free(this);
	*pThis = NULL;
}

void gen(PcodeGenerater this,enum fct x, long y, long z){
	if(this->cx>cxmax){
	printf("program too long\n");
	exit(1);
	}
	this->code[this->cx].f=x; this->code[this->cx].l=y; this->code[this->cx].a=z;
	this->cx++;
}

void printCodes(instruction* code, int len){
	for (int i = 0; i < len; i++)
	{
		printf("%3d : %s %3d %3d\n", i, fctname[code[i].f], code[i].l, code[i].a);
	}
}


void generate(PcodeGenerater this, ASTNode node)
{
	if (node != NULL)
	{
		enum AST_NODE_KINDS kind = node->kind;
		switch (kind)
		{
		case KNumber:
			gen(this, lit, 0, (int) node->val);
			break;
		case KVarExpr:
			gen(this, lod, node->sym->level ? 0 : this->level, node->sym->addr);
			break;
		case KParenExpr:
			generate(this, node->expr->oprands[0]);
			break;
		case KRelationExper:
		case KInfixExpr:
			generate(this, node->expr->oprands[0]);
			generate(this, node->expr->oprands[1]);
			gen(this, opr,0,node->expr->op);
			break;
		case KPrefixExpr:
			generate(this, node->expr->oprands[0]);
			gen(this, opr, 0, node->expr->op);
			break;
		case KProgram:
			{
				//God dame the interpreter's stack is start from 1!!!
				this->dx = BLOCK_MARK_SIZE;
				gen(this, Int, 0, BLOCK_MARK_SIZE);

				List vars = node->program->decls;
				ListItr itr = newListItr(vars, 0);
				while (hasNext(itr)) {
					generate(this, nextItem(itr));
				}
				destroyListItr(&itr);

				generate(this, node->program->main);

				// Jump to the entry address of main function
				gen(this, cal, 0, node->program->main->function->sym->addr);
				gen(this, opr, 0, 0);
			}
			break;
		case KMainFunction:
		case KFunction:
			{
				int cx1 = this->cx , dx1 = this->dx;
				gen(this, jmp, 0, 0);
				node->function->sym->addr = this->cx;

				this->dx = BLOCK_MARK_SIZE;
				gen(this, Int, 0, BLOCK_MARK_SIZE);
				//Stack block mark will cost 
				this->level++;

				generate(this, node->function->body);
				gen(this, opr, 0, 0);
				this->code[cx1].a = this->cx;

				this->level--;
				this->dx = dx1;
			}
			break;
		case KVariable:
		case KConstant:
			node->sym->addr = this->dx++;
			if (node->sym->initexpr)
				generate(this, node->sym->initexpr);
			else
				gen(this, Int, 0, 1);
			break;
		case KVarDeclStmt:
		case KConstDeclStmt:
			{
				List vars = node->vardeclstmt->vars;
				ListItr itr = newListItr(vars, 0);
				while (hasNext(itr)) {
					generate(this, nextItem(itr));
				}
				destroyListItr(&itr);
			}
			break;
		case KComposeStmt:
			{
				List vars = node->compstmt->stmts;
				ListItr itr = newListItr(vars, 0);
				while (hasNext(itr)) {
					generate(this, nextItem(itr));
				}
				destroyListItr(&itr);
			}
			break;
		case KIfStmt:
			{
				long cx1;
				generate(this, node->ifstmt->condition);
				cx1 = this->cx;
				gen(this, jpc, 0, 0);
				generate(this, node->ifstmt->thenAction);
				if (node->ifstmt->elseAction != NULL)
				{
					gen(this, jmp, 0, 0);
					this->code[cx1].a = this->cx;
					cx1 = this->cx - 1;
					generate(this, node->ifstmt->elseAction);
					this->code[cx1].a = this->cx;
				}	this->code[cx1].a = this->cx;
			}
			break;
		case KWhileStmt:
			{
				long cx1 = this->cx, cx2;
				generate(this, node->whilestmt->condition);
				cx2 = this->cx;
				gen(this, jpc, 0, 0);
				generate(this, node->whilestmt->action);
				gen(this, jmp, 0, cx1);
				this->code[cx2].a = this->cx;
			}
			break;
		case KCallExper:
			gen(this, cal, this->level, node->function->sym->addr);
			break;
		case KAssignExpr:
			{
				generate(this, node->expr->oprands[1]);
				ASTNode dst = node->expr->oprands[0];
				gen(this, sto, dst->sym->level ? 0 : this->level, dst->sym->addr);
			}
			break;
		default:
			break;
		}
	}
}