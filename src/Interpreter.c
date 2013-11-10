#include <stdio.h>
#include <stdlib.h>

enum fct {
    lit, opr, lod, sto, cal, isp, jmp, jpc, jpe, lar, sar};
// functions
enum BaseType{
	#define tpxx(a, b) TP_##a,
	#include "type.h"
	TP_LAST
	#undef tpxx
};

char mnemonic[][4]={"lit","opr","lod","sto","cal","isp","jmp","jpc","jpe","lar","sar"};

enum Operator{
#define opxx(a, b) OP_##a,
#include "op.h"
	OPLAST
#undef opxx

};

typedef struct{
    enum fct f;		// function code
    long l; 		// local/global
    long a; 		// displacement address
} instruction;

instruction *code;

typedef union{
	long i;
	float f;
	char c[4];
} memunit;

#define stacksize 500
#define staticsegment 400	//use 1000 memunit to store static data.
memunit s[stacksize];	// datastore

short debug=0,debug2=0;
/*
long base(long b, long l){
    long b1;

    b1=b;
    while (l>0){	// find base l levels down
	b1=s[b1]; l=l-1;
    }
    return b1;
}*/

long Locate(long base, long segment,long offset){
	switch(segment){
	case 0 : return offset;
	case 1 : return base+offset;
	case 2 : return staticsegment+offset;
	}
}

void interpret(){
    long p,b,t;		// program-, base-, topstack-registers
    instruction i;	// instruction register
    int ti; float tf;	// temp var for float-int cast

    printf("start C/1\n");
    t=1; b=0; p=0;
    s[1].i=0; s[2].i=0; s[3].i=0;
    do{
	i=code[p];
	if (debug2) printf("%10ld%5s%3ld%5ld\t;\t%5ld\n", p, mnemonic[i.f], i.l, i.a,t);
	p=p+1;
	switch(i.f){
	    case lit:
		t=t+1; s[t].i=Locate(b,i.l,i.a);
		break;
	    case opr:
		switch(i.a){ 	// operator
		    case OP_RET:	// return
			t=b-1; p=s[t+2].i; b=s[t+1].i;
			if (debug2) printf("  return:  t=%ld , p=%ld , b=%ld\n",t,p,b);
			break;
		    case OP_NEG:	//prefix -
			if (i.l) s[t].f=s[t].f;
			else s[t].i=-s[t].i;
			break;
		    case OP_PLUS:	//+
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i+s[t+1].i;
				break;
			case 1:
				s[t].f=s[t].i+s[t+1].f;
				break;
			case 2:
				s[t].f=s[t].f+s[t+1].i;
				break;
			case 3:
				s[t].f=s[t].f+s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_MINUS:	//-
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i-s[t+1].i;
				break;
			case 1:
				s[t].f=s[t].i-s[t+1].f;
				break;
			case 2:
				s[t].f=s[t].f-s[t+1].i;
				break;
			case 3:
				s[t].f=s[t].f-s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_MULT:	//*
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i*s[t+1].i;
				break;
			case 1:
				s[t].f=s[t].i*s[t+1].f;
				break;
			case 2:
				s[t].f=s[t].f*s[t+1].i;
				break;
			case 3:
				s[t].f=s[t].f*s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_DIV:	///
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i/s[t+1].i;
				break;
			case 1:
				s[t].f=s[t].i/s[t+1].f;
				break;
			case 2:
				s[t].f=s[t].f/s[t+1].i;
				break;
			case 3:
				s[t].f=s[t].f/s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_MOD:	//%
			t=t-1;  s[t].i=s[t].i%s[t+1].i;
			break;
		    case OP_EQL:	//==
			t=t-1; s[t].i=(s[t].i==s[t+1].i);
			break;
		    case OP_NEQ:	//!=
			t=t-1; s[t].i=(s[t].i!=s[t+1].i);
			break;
		    case OP_LSS:	//<
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i<s[t+1].i;
				break;
			case 1:
				s[t].i=s[t].i<s[t+1].f;
				break;
			case 2:
				s[t].i=s[t].f<s[t+1].i;
				break;
			case 3:
				s[t].i=s[t].f<s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_GEQ:	//>=
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i>=s[t+1].i;
				break;
			case 1:
				s[t].i=s[t].i>=s[t+1].f;
				break;
			case 2:
				s[t].i=s[t].f>=s[t+1].i;
				break;
			case 3:
				s[t].i=s[t].f>=s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_GTR:	//>
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i>s[t+1].i;
				break;
			case 1:
				s[t].i=s[t].i>s[t+1].f;
				break;
			case 2:
				s[t].i=s[t].f>s[t+1].i;
				break;
			case 3:
				s[t].i=s[t].f>s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_LEQ:	//<=
			t=t-1;
			switch (i.l)
			{
			case 0:
				s[t].i=s[t].i<=s[t+1].i;
				break;
			case 1:
				s[t].i=s[t].i<=s[t+1].f;
				break;
			case 2:
				s[t].i=s[t].f<=s[t+1].i;
				break;
			case 3:
				s[t].i=s[t].f<=s[t+1].f;
				break;
			default:
				printf("Inlegal instruction.\n");
			}
			break;
		    case OP_ANDAND:	//&&
			t=t-1; s[t].i=(s[t].i&&s[t+1].i);
			break;
		    case OP_OROR:	//||
			t=t-1; s[t].i=(s[t].i||s[t+1].i);
			break;
		    case OP_NOT:	//!
			s[t].i=!s[t].i;
			break;
		    case OP_AND:	//&
			t=t-1; s[t].i=(s[t].i&s[t+1].i);
			break;
		    case OP_OR:		//|
			t=t-1; s[t].i=(s[t].i|s[t+1].i);
			break;
		    case OP_XOR:	//^
			t=t-1; s[t].i=(s[t].i^s[t+1].i);
			break;
		    case OP_LSH:	//<<
			t=t-1; s[t].i=(s[t].i<<s[t+1].i);
			break;
		    case OP_RSH:	//>>
			t=t-1; s[t].i=(s[t].i>>s[t+1].i);
			break;
		    case OP_BITNOT:	//~
			s[t].i=~s[t].i;
			break;
	    	    case OP_CAST:	//#(cast into int/float)
			if (i.l){
				s[t].f = (float)(s[t].i);
			}
			else 	{
				s[t].i = (int)(s[t].f);
			}
			break;
		    case OP_READ:
			t++;
			switch (i.l)
			{
			case TP_FLOAT: 
				scanf("%f",&(s[t].f)); break;
			case TP_CHAR:
				scanf("%c",(char *)&(s[t].i)); break;
			case TP_INT:
			default :
				scanf("%ld",&(s[t].i)); break;
	//		case 0: scanf("%d",&s[t].i); break;
			}
			break;
		    case OP_WRITE:
			switch (i.l)
			{
			case TP_FLOAT:
				printf("%f",s[t].f); break;
			case TP_CHAR:
				printf("%c",(char)(s[t].i)); break;
			case TP_BOOL:
				if (s[t].i) printf("true");
				else printf("false");
				break;
			case TP_POINTER:	//Only the string
			{
				int j=s[t].i;
				printf("%s",(char*)&(s[j]));
//				while ((s[j].i)) printf("%c",(char)(s[j++].i));
				break;
			}
			case TP_INT:
			default :
				printf("%ld",s[t].i); break;
	//		case 0: scanf("%d",&s[t].i); break;
			}
			t--;//Should I do this?
			break;
//		    case OP_CALL:
//		    case OP_DEREF:
//		    case OP_ADDRESS:
		}
		break;
	    case lod:
		t=t+1; s[t].i=s[Locate(b,i.l,i.a)].i;
		if (debug) printf("  load:  s[%3ld] = s[%3ld] = %ld(%6.2f)\n",t,Locate(b,i.l,i.a), s[t].i,s[t].f);
		break;
	    case lar:
		//*sp=*(*sp+base*local+offset);
		if (debug) printf("  load:  s[%3ld] = s[%3ld] = ",t,Locate(b,i.l,i.a)+s[t].i);
		s[t].i = s[s[t].i+Locate(b,i.l,i.a)].i;
		if (debug) printf("%ld(%6.2f)\n",s[t].i,s[t].f);
		break;
	    case sto:
		s[Locate(b,i.l,i.a)].i=s[t].i; 
		if (debug) printf("  store: s[%3ld] = %ld(%6.2f)\n",Locate(b,i.l,i.a), s[t].i,s[t].f);
//		t=t-1;	//delete this to enable chain assign
		break;
	    case sar:
		s[s[t].i+Locate(b,i.l,i.a)].i = s[t-1].i;
		if (debug) printf("  store: s[%3ld] = %ld(%6.2f)\n",s[t].i+Locate(b,i.l,i.a), s[t-1].i,s[t-1].f);
		t--;
		break;
	    case cal:		// generate new block mark
		s[t+1].i=b; s[t+2].i=p;
		b=t+1; p=i.a;
		break;
	    case isp:
		t=t+i.a;
		break;
	    case jmp:
		p=i.a;
		break;
	    case jpc:
		if(!s[t].i){
		    p=i.a;
		}
		if(!i.l) t=t-1;
		break;
	    case jpe:
		if(s[t].i){
		    p=i.a;
		}
		if(!i.l) t=t-1;
		break;
	}
    }while(p!=0);
    printf("end C/1\n");
}

main(int argc,char** argv){
	FILE* infile;
	long Length,buffsize;
	char* buffer;
	if (argc==1) printf("usage: C0linker taget.o\n");
	if((infile=fopen(argv[1],"r"))==0)
	{
		printf("File %s can't open.\n",argv[1]);
		fclose(infile);
		exit(1);
	}
	if ((argc>2)&&(strcmp(argv[2],"-d")==0))
		debug=1;
	if ((argc>2)&&(strcmp(argv[2],"-d2")==0))
		debug2=debug=1;
	fread(&buffsize,sizeof(long),1,infile);
	fread(&s[staticsegment],sizeof(memunit),buffsize,infile);
/*	buffer=(char*)malloc(sizeof(char)*buffsize);
	fread(buffer,sizeof(char),buffsize,infile);
	printf("StaticMemery[%ld]:\n",buffsize);
	int i;
	for (i=0;i<buffsize;i++)
	{
		s[staticsegment+i].i=buffer[i];
		if (buffer[i])	putchar(buffer[i]);
		else putchar('$');
	}
	printf("\n");*/
//	getchar();
	fread(&Length,sizeof(long),1,infile);
	code=(instruction*)malloc(sizeof(instruction)*Length);
	fread(code,sizeof(instruction),Length,infile);
	fclose(infile);
	interpret();
}
