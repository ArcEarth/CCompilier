#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

extern char *opname[];
extern char *typename[];
extern char StaticMemery[];
extern int smp;
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
/*
typedef struct{
	Type retunval;
	List paramenters;
//	ASTNode exec;
} *Function;
*/

Type Int,Float,Char,Bool,Void,ConstBool,ConstInt,ConstFloat,ConstChar,String,ConstString; //The Global Var to pointer to the base types.

void TypeSystemInitilize()
{
	NEW0(Int);
	Int->kind = TP_INT;
	Int->size = 1;
	NEW0(Char);
	Char->kind = TP_CHAR;
	Char->size = 1;
	NEW0(Float);
	Float->kind = TP_FLOAT;
	Float->size = 1;
	NEW0(Bool);
	Bool->kind = TP_BOOL;
	Bool->size = 1;
	NEW0(Void);
	Void->kind = TP_VOID;
	Void->size = 0;
	NEW0(ConstInt);
	ConstInt->kind = TP_INT;
	ConstInt->size = 1;
	ConstInt->constant = 1;
	NEW0(ConstFloat);
	ConstFloat->kind = TP_FLOAT;
	ConstFloat->size = 1;
	ConstFloat->constant = 1;
	NEW0(ConstBool);
	ConstBool->kind = TP_BOOL;
	ConstBool->size = 1;
	ConstBool->constant = 1;
	NEW0(ConstChar);
	ConstChar->kind = TP_CHAR;
	ConstChar->size = 1;
	ConstChar->constant = 1;
	String=newPointer(ConstChar);
	String->staticdata = 1;	//the address lead to a static data segment but not the stack
	ConstString=newConst(String);
	ConstString->staticdata = 1;
//	printf("%d,%d\n",ConstString->kind,ConstString->pointer->base->kind);
//	printf("%d,%d,%d,%d\n",Void->kind,Bool->kind,Int->kind,Float->kind);
}

void TypeSizeFix(Type p){
	switch (p->kind){
	case TP_INT:
	case TP_CHAR:
	case TP_BOOL:
	case TP_FLOAT:
		p->size = 1;
		break;
	case TP_VOID:
		p->size = 0;
		break;
	case TP_POINTER:
		TypeSizeFix(p->pointer->base);
		p->size = 1;
		break;
	case TP_ARRAY:
		TypeSizeFix(p->array->base);
		p->size = p->array->base->size * p->array->length;
		break;
	default:
		p->size = 1;
	}
}

Type newConst(Type base)
{
	switch(base->kind)
	{
	case TP_INT:
		return ConstInt;
	case TP_CHAR:
		return ConstChar;
	case TP_BOOL:
		return ConstBool;
	case TP_FLOAT:
		return ConstFloat;
	case TP_VOID:
		return Void;
	default:
		break;
	}
	Type new;
	NEW0(new);
	new->kind = base->kind;
	new->size = base->size;
	new->constant = 1;
	switch(base->kind)
	{	
	case TP_POINTER:
		new->pointer = base->pointer;
		break;
	case TP_ARRAY:
		new->array = base->array;
		break;
	default:
		return Void;
	}
	return new;
}

Type newStatic(Type base)
{
	Type new;
	NEW0(new);
	new->kind = base->kind;
	new->size = base->size;
	new->constant = base->constant;
	new->staticdata = 1;
	switch(base->kind)
	{	
	case TP_POINTER:
		new->pointer = base->pointer;
		break;
	case TP_ARRAY:
		new->array = base->array;
		break;
	}
	return new;
}

Type newPointer(Type base)
{
	Type new;
	NEW0(new);
	new->kind = TP_POINTER;
	Pointer newp;
	NEW0(newp);
	newp->base = base;
	new->pointer = newp;
	new->size = 1;
	return new;
}

Type newArray(Type base,int length)
{
	Type new;
	NEW0(new);
	new->kind = TP_ARRAY;
	Array newa;
	NEW0(newa);
	newa->base = base;
	newa->length = length;
	new->array = newa;
	new->size = length*base->size;
	return new;
}

Type tracePointer(Type cap)
{
	//the size of parent type will be remain(invailed)
	Type parent = NULL,base = cap;
	while ((base->kind==TP_ARRAY)||(base->kind==TP_POINTER)){
		parent = base;
		if (base->kind==TP_ARRAY)
			base = base->pointer->base;
		else 
			base = base->array->base;
	}	
	Type new;
	NEW0(new);
	new->kind = TP_POINTER;
	Pointer newp;
	NEW0(newp);
	newp->base = base;
	new->pointer = newp;
	new->size = 1;
	if (parent == NULL) return new;
	else {
		if (parent->kind==TP_POINTER)
			parent->pointer->base = new;
		else
			parent->array->base = new;
		TypeSizeFix(cap);
		return cap;
	}
}

Type traceArray(Type cap,Value length)
{
//	printf("NewArrayStart!\n");
	if (!TypeMatch(length.type,Int)){
		printf("Error : Array length must be int!\n");
		return Void;
	}
//	printf("NewArrayS1!\n");
	Type parent = NULL,base = cap;
	while ((base->kind==TP_ARRAY)||(base->kind==TP_POINTER)){
		parent = base;
//		parent->size *= length.Int;
		if (base->kind==TP_ARRAY)
			base = base->pointer->base;
		else 
			base = base->array->base;
	}	
	Type new;
	NEW0(new);
	new->kind = TP_ARRAY;
	Array newa;
	NEW0(newa);
	newa->length = length.Int;
//	printf("NewArrayS2!\n");
	newa->base = base;
//	printf("NewArrayS3!\n");
	new->array = newa;
	new->size = base->size * length.Int;
//	printf("NewArrayFinish!\n");
	if (parent == NULL) return new;
	else {
		if (parent->kind==TP_POINTER)
			parent->pointer->base = new;
		else
			parent->array->base = new;
		TypeSizeFix(cap);
		return cap;
	}
}

void dumpType(Type t)
{
	if (t->constant) printf("const ");
	switch (t->kind)
	{
	case TP_INT:
		printf("int");
		return;
	case TP_CHAR:
		printf("char");
		return;
	case TP_VOID:
		printf("void");
		return;
	case TP_BOOL:
		printf("bool");
		return;
	case TP_FLOAT:
		printf("float");
		return;
	case TP_ARRAY:
		printf("[%d]",t->array->length);
		dumpType(t->array->base);
		return;
	case TP_POINTER:
		printf("*");
		if (t->pointer->base->kind==TP_ARRAY)
		{
			printf("(");
			dumpType(t->pointer->base);
			printf(")");
		} else
			dumpType(t->pointer->base);
		return;
	default:
		printf("unkowntype %d",t->kind);
	}
}

//The struction equal judge
bool TypeMatch(Type A,Type B){
	while (A->kind==B->kind)
	{
		switch (A->kind) {
		case TP_VOID:
		case TP_BOOL:
		case TP_CHAR:
		case TP_INT:
		case TP_FLOAT:
			return 1;
		case TP_POINTER:
			A=A->pointer->base;
			B=B->pointer->base;
			break;
		case TP_ARRAY:
			if (A->array->length!=B->array->length)
				return 0;
			A=A->array->base;
			B=B->array->base;
			break;
		default:
			return 0;
		}
	}
	return 0;
}

Type TypeCheck(int op,Type leftT,Type rightT)
{
	int left,right;
	left = leftT->kind;
	right = rightT->kind;
//	printf("TypeCheck[%s,%s,%s]\n",opname[op],typename[left],typename[right]);
	if ((left==TP_VOID)||((op!=OP_NEG)&&(op!=OP_NOT)&&(op!=OP_DEREF)&&(op!=OP_ADDRESS)&&(op!=OP_REF)&&(op!=OP_CAST)&&(op!=OP_READ)&&(op!=OP_WRITE)&&(right==TP_VOID)))
	{
		printf("Error : Void type expression can not invole in operatting.\n");
		 return Void;
	}
	if ((op<OP_INDEX)&&(max(left,right)>TP_FLOAT)) 
	{
		printf("Error : There is no overload for expression : ");
		dumpType(leftT);
		printf(" %s ",opname[op]);
		dumpType(rightT);
		printf("\n");
		return Void;
	}
	switch (op){
	case OP_NEG:
		if (left==TP_BOOL)
		{
			printf("Error : Not operator for bool is '!' but not '-'.\n");
		}
		return leftT;
	case OP_PLUS:
	case OP_MINUS:
	case OP_MULT:
	case OP_DIV:
		if (left==TP_ARRAY||right==TP_ARRAY)
			printf("Error : Array can not take into arithmeic computes.\n");
		if (left>right) return leftT;
		else return rightT;
	case OP_MOD:
		if (left==TP_ARRAY||right==TP_ARRAY)
			printf("Error : Array can not take into arithmeic computes.\n");
		if ((left>TP_INT)||(right>TP_INT))
		{
			printf("Error : Mod operater can not handle over-int type");
			return Int;
//			return TP_VOID;
		}
		return Int;
	case OP_ASGN:
		if (left==TP_ARRAY)
		{
			printf("Error : Can not Assign to a array!\n");
		}
		if (left<right)
		{
//			if (left==TP_FLOAT)&&(right<=TP_INT)
			printf("Warning : Cast %s into %s may be damageble.\n",typename[right],typename[left]);
		}
		if (leftT->constant)
		{
			printf("Error : Can not assign to a const type.\n");		}
		return leftT;
	case OP_EQL:
	case OP_NEQ:
		if (max(left,right)==TP_FLOAT)
			printf("Warning : Using float in EQL/NEQ condition may cause unpredictive issue.\n");
	case OP_LSS:
	case OP_GEQ:
	case OP_GTR:
	case OP_LEQ:
		return Bool;
	case OP_ANDAND:
	case OP_OROR:
		if (max(left,right)==TP_FLOAT)
			printf("Error : Condition operator can not handle float type.");
		return Bool;
	case OP_NOT:
		if (left==TP_FLOAT) 
			printf("Error : 'Not' operator can not handle float type.\n");
		return Bool;
	case OP_AND:
	case OP_OR:
	case OP_XOR:
	case OP_LSH:
	case OP_RSH:
		if (max(left,right)==TP_FLOAT)
			printf("Error : Bit operator can not handle float type.");
		return Int;		
	case OP_INDEX:
		if (((left!=TP_ARRAY)&&(left!=TP_POINTER))||(right!=TP_INT))
		{
			printf("Error : [] operator must use with  type assert array[int].\n");
		}
		return leftT->array->base;
	case OP_CALL:
		return Void;
	case OP_DEREF:
		if (left!=TP_POINTER)
		{
			printf("Error : Dereference operator * can not handle type ");
			dumpType(leftT);
			printf(" .\n");
			return Void;
		}
		return leftT->pointer->base;
	case OP_ADDRESS:
		return newPointer(leftT);
	case OP_REF:
		return leftT;
	case OP_CAST:
		if (left==TP_FLOAT) return Int;
		else return Float;
	case OP_READ:
		if (left!=TP_POINTER)
		{
			printf("Error : Read operator Must hold a pointer type.\n");
			return Int;
		}
		return leftT->pointer->base;
	case OP_WRITE:
		switch (left){
		case TP_BOOL:
		case TP_CHAR:
		case TP_INT:
		case TP_FLOAT:
			return leftT;
		case TP_ARRAY:
		case TP_POINTER:
			if (leftT->pointer->base->kind==TP_CHAR) return String;
			printf("Warning : Write operator Can not hold premium type.It'will treat as int.\n");
			return Int;
		}
	default:
		printf("Error : There is no overload for expression : ");
		dumpType(leftT);
		printf(" %s ",opname[op]);
		dumpType(rightT);
		printf("\n");
		return Void;
	}
}

Type getType(ASTNode node)
{
//	printf("getType\n");
	if (node==NULL) return Void;
	switch (node->kind)
	{
	case KValue:
		return node->val.type;
	case KFunctionDecl:
		return node->funcl->sym->type;
	case KDeRef:
	case KIndex:
	case KAdrExp:
	case KPrefixExp:
	case KInfixExp:
	case KAssignExp:
	case KParenExp:
		return node->exp->type;
	case KFunCall:
		return node->funcall->sym->type;
	case KVar:
//	case KConst:
	case KRef:
		return node->sym->type;
	default:
		printf("Unhandled type : %x\n",node->kind);
		return Void;
	}
//	printf("getTypeFinish\n");
}

PrintVal(Value *v){
	switch (v->type->kind)
	{
	case TP_INT:
		printf("%d", v->Int);
		break;
	case TP_CHAR:
//		putchar(v->Char);
		printf("'%c'", v->Char);
		break;
	case TP_FLOAT:
		printf("%f", v->Float);
		break;
	case TP_BOOL:
		if (v->Bool) 
			printf("true");
		else	printf("false");
		break;
	case TP_ARRAY:
		{
			ListItr Itr=newListItr(v->Array,0);
			printf("{");
			while (hasNext(Itr)){
				PrintVal((Value*)nextItem(Itr));
				printf(",");
			}
			printf("\b}");
			break;
		}
	default:
		if (TypeMatch(v->type,String))
		{
			printf("\"%d\"",v->Int);
//			printf("\"%s\"",v->Int+StaticMemery);
			break;
		}
		printf("Error!");
	}
}

