#ifndef _PCODE_GEN_H_
#define _PCODE_GEN_H_

#include "pcode.h"

#define cxmax 1000

typedef struct pcodegenerater
{
	long cx, dx, level;
	instruction *code;
} *PcodeGenerater;

PcodeGenerater newPcodeGenerater();
void destroyPcodeGenerater(PcodeGenerater *pThis);
void generate(PcodeGenerater, ASTNode);
void printCodes(instruction* code, int len);

#endif // !_PCODE_GEN_H_
