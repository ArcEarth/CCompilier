/*
* The shell of the compilier, interpret the command line arguments, output to file
*/

#include "common.h"
#include "pcodegen.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>


//SymbolTable symtab = NULL; //Strong defineation
//ASTree ast = NULL;
extern struct BitCodes codeGen(ASTree ast);
extern ASTree parse();

int main(int argc, char *argv [])
{

	FILE* outfile = NULL;
	char infilename[256], outfilename[256];

	// Section of parsing command line arguments
	// Added by Yupeng
	int k, flag = 0;
	for (k = 1; k < argc; k++)
	{
		if (argv[k][0] != '-') {
			strcpy(infilename, argv[1]);
			freopen(infilename, "r", stdin);
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
				{
					strcpy(outfilename, argv[k]);
					outfile = fopen(outfilename, "wb");
				}
			}
		}
	}
	if (!flag) printf("Warning : haven't decide a source file, system will take the standard input as source.\n");
	if (outfile == NULL)
		outfile = fopen("a.o", "wb");
	// End of Arguments parsing

	printf("Parsing ...\n");
	ASTree ast = parse();
	printf("\n\nDump the program from the generated AST:\n");
	dumpASTNode(ast->root, 0);

	// Section of code generate and output to file
	// Added by Yupeng Zhang
	printf("\n\nPCode generated from the generated AST:\n");
	PcodeGenerater generator = newPcodeGenerater();
	generate(generator, ast->root);

	int result;
	printCodes(generator->code, generator->cx);
	result = fwrite(&generator->cx, sizeof(long), 1, outfile);
	result = fwrite(generator->code, sizeof(instruction), generator->cx, outfile);
	fclose(outfile);
	
	//freopen(outfilename, "r", outfile);

	//system("PAUSE");
	//printf("=====================\n");
	//FILE* infile = fopen(outfilename, "r");
	//if (outfile){
	//	int len = fread(&len, sizeof(long), 1, outfile);
	//	instruction* code = (instruction*) malloc(sizeof(instruction) *(len + 1));
	//	fread(code, sizeof(instruction), len, outfile);
	//	printCodes(code, len);
	//	fclose(outfile);
	//	free(code);
	//}

	// End of Section
	destroyAST(&ast);
	destroyPcodeGenerater(&generator);
	printf("\n\nFinished destroying AST.\n");

	return(0);
}
