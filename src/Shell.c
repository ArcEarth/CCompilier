#include "common.h"
#include <stdio.h>

Table symtab;
ASTTree ast;
FILE *outfile;

extern int yyparse();

int main(int argc, char *argv[])
{

	symtab = newTable();
	ast = newAST();
	TypeSystemInitilize();
	int k, flag = 0;
	for (k = 1; k<argc; k++)
	{
		if (argv[k][0] != '-') {
			freopen(argv[1], "r", stdin);
			flag = 1;
		}
		else {
			if (argv[k][1] == 'o') {
				k++;
				//				printf("outputfile : argv[%d] : %s\n",k,argv[k]);
				if (k == argc) {
					printf("Error : Inlegal command. There must be a param after -o option.\n");
				}
				else
					outfile = fopen(argv[k], "w");
			}
		}
	}
	if (!flag) printf("Warning : haven't decide a source file, system will take the standard input as source.\n");
	if (outfile == NULL)
		outfile = fopen("a.o", "w");
	printf("Parsing ...\n");
	yyparse();
	printf("\n\nDump the program from the generated AST:\n");
	dumpAST(ast->root);
	GenAST(ast->root);
	destroyAST(&ast->root);
	destroyTable(&symtab);
	return(0);
}
