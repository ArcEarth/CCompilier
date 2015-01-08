#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pcode.h"


#define STACK_MAX 2000

instruction* code;
long s[STACK_MAX];
int debug = 0;

long Locate(long base, long segment, long offset){
	switch (segment){
	case 0: return offset;
	case 1: return base + offset;
	default: return offset;
	}
}

long base(long b, long l)
{
	long b1;

	b1 = b;
	while (l>0)	// find base l levels down
	{
		b1 = s[b1];
		l = l - 1;
	}
	return b1;
}

void interpret()
{
	long p,b,t;		// program-, base-, topstack-registers
	instruction i;	// instruction register
	
	printf("start C0\n");
	t=0; b=1; p=0;
	s[1]=0; s[2]=0; s[3]=0;
	do
	{
		if (t > 100){
			printf("stack overflow.\n");
			return;
		}
		i=code[p];
		if (debug >= 2)
			printf("%3d : %s %3d %3d ; [%3d] = %3d\n", p, fctname[i.f], i.l, i.a, t, s[t]);
		p = p + 1;
		switch(i.f)
		{
		case lit:
			t=t+1; s[t]=i.a;
			break;
		case opr:
			switch(i.a) 	// operator
			{
			case 0:	// return
				t=b-1; p=s[t+3]; b=s[t+2];
				break;
			case 1:
				s[t]=-s[t];
				break;
			case 2:
				t=t-1; s[t]=s[t]+s[t+1];
				break;
			case 3:
				t=t-1; s[t]=s[t]-s[t+1];
				break;
			case 4:
				t=t-1; s[t]=s[t]*s[t+1];
				break;
			case 5:
				t=t-1; s[t]=s[t]/s[t+1];
				break;
			case 6:
				t = t - 1; s[t] = s[t] % s[t + 1];
				break;
			case 8:
				t=t-1; s[t]=(s[t]==s[t+1]);
				break;
			case 9:
				t=t-1; s[t]=(s[t]!=s[t+1]);
				break;
			case 10:
				t=t-1; s[t]=(s[t]<s[t+1]);
				break;
			case 11:
				t=t-1; s[t]=(s[t]>=s[t+1]);
				break;
			case 12:
				t=t-1; s[t]=(s[t]>s[t+1]);
				break;
			case 13:
				t=t-1; s[t]=(s[t]<=s[t+1]);
				break;
			}
			break;
		case lod:
			t=t+1; s[t]=s[base(b,i.l)+i.a];
			if (debug >= 1)
				printf(" lod : [%3d] = %3d\n", base(b, i.l) + i.a, s[t]);
			break;
		case sto:
			s[base(b,i.l)+i.a]=s[t];
			printf(" sto : [%3d] = %3d\n", base(b, i.l) + i.a, s[t]);
			t = t - 1;
			break;
		case cal:		// generate new block mark
			s[t+1]=base(b,i.l); s[t+2]=b; s[t+3]=p;
			b=t+1; p=i.a;
			break;
		case Int:
			t=t+i.a;
			break;
		case jmp:
			p=i.a;
			break;
		case jpc:
			if(s[t]==0)
				p=i.a;
			t=t-1;
			break;
		default:
			printf("invaliad instruction.");
			return;
		}
	}while(p!=0);
	printf("end C0\n");
}

void printCodes(instruction* code , int len){
	for (int i = 0; i < len; i++)
	{
		printf("%3d : %s %3d %3d\n", i, fctname[code[i].f], code[i].l, code[i].a);
	}
}

int main(int argc, char *argv[])
{	
	FILE* infile = NULL;
	char infilename[256];
	if (argc == 1) {
		printf("please input source program file name: ");
		scanf("%s", infilename);
		printf("\n");
	}
	else {
		strcpy(infilename, argv[1]);
	}

	if ((argc>2) && (strcmp(argv[2], "-d") == 0))
		debug = 1;
	if ((argc>2) && (strcmp(argv[2], "-d2") == 0))
		debug = 2;

	if((infile=fopen(infilename,"rb"))==NULL)
	{
		printf("File %s can't be opened.\n", infilename);
		exit(1);
	}
	//long i=0;

	long Length = 0;
	// Read the length of this array ("blob")
	fread(&Length, sizeof(long), 1, infile);
	// allocate the space for save it.
	code = (instruction*) malloc(sizeof(instruction) *(Length));
	// Read the array
	fread(code, sizeof(instruction), Length, infile);
	fclose(infile);

	if (debug >= 1)
		printCodes(code, Length);

	//while(!feof(infile))
	//{
	//	fread(&code[i],sizeof(instruction),1,infile);
	//	i++;
	//}
	interpret();
	//fclose(infile);
	return 1;
}


















