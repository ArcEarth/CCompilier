#include "stdafx.h"
#include "pcode.h"
#include "interpreter.h"

void C1::PCode::Interpreter::Interpret(const CodeDome& Codedome)
{
	instruction i;	// instruction register
	const instruction* code = &Codedome.Instruction(0);
	std::copy(Codedome.Datas().begin(), Codedome.Datas().end(), DS);
	//	int ti; float tf;	// temp var for float-int cast
	printf("start C/1\n");
	SP = 1; BP = 0; IR = 0;
	//SS[1].i = 0; SS[2].i = 0; SS[3].i = 0;
	SS[0].i = 0; SS[1].i = -1; //SS[3].i = 0;
	do{
		if (SP < 0) {
			printf("Runtime error : Stack underflow\n");
			break;
		}
		if (SP > stacksize) {
			printf("Runtime error : Stack overflow\n");
			break;
		}
		i = code[IR];
		if (debug2) printf("%10ld%5s%3ld%5ld\t;\t%5ld\n", IR, fctname[i.f], i.l, i.a, SP);
		IR = IR + 1;
		switch (i.f){
		case lit:
			SP = SP + 1; SS[SP].i = Locate(BP, i.l, i.a);
			break;
		case opr:
			switch (i.a){ 	// operator
			case OP_RET:	// return
				SP = BP - 1; IR = SS[SP + 2].i; BP = SS[SP + 1].i;
				if (debug2) printf("  return:  SP=%ld , IR=%ld , BP=%ld\n", SP, IR, BP);
				break;
			case OP_NEG:	//prefix -
				if (i.l) SS[SP].f = SS[SP].f;
				else SS[SP].i = -SS[SP].i;
				break;
			case OP_ADD:	//+
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i + SS[SP + 1].i;
					break;
				case 1:
					SS[SP].f = SS[SP].i + SS[SP + 1].f;
					break;
				case 2:
					SS[SP].f = SS[SP].f + SS[SP + 1].i;
					break;
				case 3:
					SS[SP].f = SS[SP].f + SS[SP + 1].f;
					break;
				default:
					printf("illegal instruction.\n");
				}
				break;
			case OP_SUB:	//-
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i - SS[SP + 1].i;
					break;
				case 1:
					SS[SP].f = SS[SP].i - SS[SP + 1].f;
					break;
				case 2:
					SS[SP].f = SS[SP].f - SS[SP + 1].i;
					break;
				case 3:
					SS[SP].f = SS[SP].f - SS[SP + 1].f;
					break;
				default:
					printf("Inlegal instruction.\n");
				}
				break;
			case OP_MUL:	//*
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i*SS[SP + 1].i;
					break;
				case 1:
					SS[SP].f = SS[SP].i*SS[SP + 1].f;
					break;
				case 2:
					SS[SP].f = SS[SP].f*SS[SP + 1].i;
					break;
				case 3:
					SS[SP].f = SS[SP].f*SS[SP + 1].f;
					break;
				default:
					printf("illegal instruction.\n");
				}
				break;
			case OP_DIV:	///
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i / SS[SP + 1].i;
					break;
				case 1:
					SS[SP].f = SS[SP].i / SS[SP + 1].f;
					break;
				case 2:
					SS[SP].f = SS[SP].f / SS[SP + 1].i;
					break;
				case 3:
					SS[SP].f = SS[SP].f / SS[SP + 1].f;
					break;
				default:
					printf("illegal instruction.\n");
				}
				break;
			case OP_MOD:	//%
				SP = SP - 1;  SS[SP].i = SS[SP].i%SS[SP + 1].i;
				break;
			case OP_EQL:	//==
				SP = SP - 1; SS[SP].i = (SS[SP].i == SS[SP + 1].i);
				break;
			case OP_NEQ:	//!=
				SP = SP - 1; SS[SP].i = (SS[SP].i != SS[SP + 1].i);
				break;
			case OP_LSS:	//<
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i < SS[SP + 1].i;
					break;
				case 1:
					SS[SP].i = SS[SP].i < SS[SP + 1].f;
					break;
				case 2:
					SS[SP].i = SS[SP].f < SS[SP + 1].i;
					break;
				case 3:
					SS[SP].i = SS[SP].f < SS[SP + 1].f;
					break;
				default:
					printf("Inlegal instruction.\n");
				}
				break;
			case OP_GEQ:	//>=
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i >= SS[SP + 1].i;
					break;
				case 1:
					SS[SP].i = SS[SP].i >= SS[SP + 1].f;
					break;
				case 2:
					SS[SP].i = SS[SP].f >= SS[SP + 1].i;
					break;
				case 3:
					SS[SP].i = SS[SP].f >= SS[SP + 1].f;
					break;
				default:
					printf("illegal instruction.\n");
				}
				break;
			case OP_GTR:	//>
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i > SS[SP + 1].i;
					break;
				case 1:
					SS[SP].i = SS[SP].i > SS[SP + 1].f;
					break;
				case 2:
					SS[SP].i = SS[SP].f > SS[SP + 1].i;
					break;
				case 3:
					SS[SP].i = SS[SP].f > SS[SP + 1].f;
					break;
				default:
					printf("Inlegal instruction.\n");
				}
				break;
			case OP_LEQ:	//<=
				SP = SP - 1;
				switch (i.l)
				{
				case 0:
					SS[SP].i = SS[SP].i <= SS[SP + 1].i;
					break;
				case 1:
					SS[SP].i = SS[SP].i <= SS[SP + 1].f;
					break;
				case 2:
					SS[SP].i = SS[SP].f <= SS[SP + 1].i;
					break;
				case 3:
					SS[SP].i = SS[SP].f <= SS[SP + 1].f;
					break;
				default:
					printf("Inlegal instruction.\n");
				}
				break;
			case OP_ANDAND:	//&&
				SP = SP - 1; SS[SP].i = (SS[SP].i&&SS[SP + 1].i);
				break;
			case OP_OROR:	//||
				SP = SP - 1; SS[SP].i = (SS[SP].i || SS[SP + 1].i);
				break;
			case OP_NOT:	//!
				SS[SP].i = !SS[SP].i;
				break;
			case OP_AND:	//&
				SP = SP - 1; SS[SP].i = (SS[SP].i&SS[SP + 1].i);
				break;
			case OP_OR:		//|
				SP = SP - 1; SS[SP].i = (SS[SP].i | SS[SP + 1].i);
				break;
			case OP_XOR:	//^
				SP = SP - 1; SS[SP].i = (SS[SP].i^SS[SP + 1].i);
				break;
			case OP_LSH:	//<<
				SP = SP - 1; SS[SP].i = (SS[SP].i << SS[SP + 1].i);
				break;
			case OP_RSH:	//>>
				SP = SP - 1; SS[SP].i = (SS[SP].i >> SS[SP + 1].i);
				break;
			case OP_REVERSE:	//~
				SS[SP].i = ~SS[SP].i;
				break;
			case OP_CAST:	//#(cast into int/float)
				if (i.l){
					SS[SP].f = (float) (SS[SP].i);
				}
				else 	{
					SS[SP].i = (int) (SS[SP].f);
				}
				break;
			case OP_READ:
				SP++;
				switch (i.l)
				{
				case TP_FLOAT:
					scanf("%f", &(SS[SP].f)); break;
				case TP_CHAR:
					scanf("%c", (char *) &(SS[SP].i)); break;
				case TP_INT:
				default:
					scanf("%ld", &(SS[SP].i)); break;
					//		case 0: scanf("%d",&s[SP].i); break;
				}
				break;
			case OP_WRITE:
				switch (i.l)
				{
				case TP_FLOAT:
					printf("%f", SS[SP].f); break;
				case TP_CHAR:
					printf("%c", (char) (SS[SP].i)); break;
				case TP_BOOL:
					if (SS[SP].i) printf("true");
					else printf("false");
					break;
				case TP_STRING:	//Only the string
				{
					int j = SS[SP].i;
					char c = SS[j].c[0];
					while (c != '\0') {
						printf("%c", c);
						c = SS[++j].c[0];
					}
					break;
					//printf("%s", (char*) &(SS[j]));
					////				while ((s[j].i)) printf("%c",(char)(s[j++].i));
					//break;
				}
				case TP_INT:
				default:
					printf("%ld", SS[SP].i); break;
					//		case 0: scanf("%d",&s[SP].i); break;
				}
				SP--;//Should I do this?
				break;
				//		    case OP_CALL:
				//		    case OP_DEREF:
				//		    case OP_ADDRESS:
			}
			break;
		case lod:
			SP = SP + 1; SS[SP].i = SS[Locate(BP, i.l, i.a)].i;
			if (debug) printf("  load:  s[%3ld] = s[%3ld] = %ld(%6.2f)\n", SP, Locate(BP, i.l, i.a), SS[SP].i, SS[SP].f);
			break;
		case lar:
			//*sp=*(*sp+base*local+offset);
			if (debug) printf("  load:  s[%3ld] = s[%3ld] = ", SP, Locate(BP, i.l, i.a) + SS[SP].i);
			SS[SP].i = SS[SS[SP].i + Locate(BP, i.l, i.a)].i;
			if (debug) printf("%ld(%6.2f)\n", SS[SP].i, SS[SP].f);
			break;
		case sto:
			SS[Locate(BP, i.l, i.a)].i = SS[SP].i;
			if (debug) printf("  store: s[%3ld] = %ld(%6.2f)\n", Locate(BP, i.l, i.a), SS[SP].i, SS[SP].f);
			//		t=t-1;	//delete this to enable chain assign
			break;
		case sar:
			SS[SS[SP].i + Locate(BP, i.l, i.a)].i = SS[SP - 1].i;
			if (debug) printf("  store: s[%3ld] = %ld(%6.2f)\n", SS[SP].i + Locate(BP, i.l, i.a), SS[SP - 1].i, SS[SP - 1].f);
			SP--;
			break;
		case cal:		// generate new block mark
			{
				if (debug2) printf("  cal:  SP=%ld , IR=%ld , BP=%ld\n", SP, IR, BP);
				if (i.l == 0)
				{
					SS[SP + 1].i = BP; SS[SP + 2].i = IR;
					BP = SP + 1;
					IR = i.a;
				}
				else
				{
					long tar = SS[SP--].i;
					SS[SP + 1].i = BP; SS[SP + 2].i = IR;
					BP = SP + 1;
					IR = tar;
				}
			}
			break;
		case isp:
			SP = SP + i.a;
			break;
		case jmp:
			IR = i.a;
			break;
		case jpc:
			if (!SS[SP].i){
				IR = i.a;
			}
			if (!i.l) SP = SP - 1;
			break;
		case jpe:
			if (SS[SP].i){
				IR = i.a;
			}
			if (!i.l) SP = SP - 1;
			break;
		}
	} while (IR >= 0);
	printf("end C/1\n");
}

void C1::PCode::Interpreter::SetDebugLevel(int level)
{
	debug = debug2 = false;
	if (level >= 1) debug = true;
	if (level >= 2) debug2 = true;
}

inline long C1::PCode::Interpreter::Locate(long base, long segment, long offset)
{
	switch (segment){
	case PCode::SS: return offset;
	case PCode::SS_BP: return base + offset;
	case PCode::DS: return DS-SS + offset;
	default: return -1; // Error
	}
}
