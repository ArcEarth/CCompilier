/*
 * expr.lex : Scanner for a simple
 *            expression parser.
 */

%{
#include "common.h"
#include "C1.tab.h"
#include "string.h"
#include "stdlib.h"
int yycolumn = 1;
extern Type Int,Float,Char,Bool,Void,ConstBool,ConstInt,ConstFloat,ConstChar,String,ConstString;
extern Table symtab;
#define STATICSIZE 2048
memunit StaticMemery[ STATICSIZE ];
int smp=0;
Loc PositionPointer = (Loc)&yylloc;
#define NUMAXLEN 10
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng;
%}

%option yylineno
%option noyywrap

%x COMMENT
%x LINECOMMENT

identifier [A-Za-z_][A-Za-z_0-9]*
character  \'([^'\n])+\'
bad_chr    \'([^'\n])
string	   \"([^"\n])+\"
bad_string \"([^"\n])+

%%
"//"                {
			BEGIN(LINECOMMENT);
		    }
<LINECOMMENT>[^\n]+ ;
<LINECOMMENT><<EOF>>    printf("Error : File end during comments.\n");
<LINECOMMENT>\n     {
			BEGIN(INITIAL);
		    }

"/*"                {
			BEGIN(COMMENT);
		    }
<COMMENT>[^*/]+	    ;
<COMMENT><<EOF>>    printf("Error : File end during comments.\n");
<COMMENT>"*/"       {
			BEGIN(INITIAL);
		    }
<COMMENT>[*/]	    ;

[\t ]*     //printf("%s",yytext);/* throw away whitespace */
"{"	   return(BEGINSYM);
"}"	   return(ENDSYM);
"["	   return(LBRACK);
"]"	   return(RBRACK);
"const"	   return(CONSTSYM);
"static"	   return(STATICSYM);
"typedef"	   return(TYPEDEFSYM);
"float"	   {yylval.tval = Float; return(BASETYPE);}
"int"	   {yylval.tval = Int; return(BASETYPE);}
"char"	   {yylval.tval = Char; return(BASETYPE);}
"bool"	   {yylval.tval = Bool; return(BASETYPE);}
"void"	   {yylval.tval = Void; return(BASETYPE);}
"true"	   {
	     yylval.val.type = ConstBool;
	     yylval.val.Bool = 1;
	     return(NUMBER);
	   }
"false"	   {
	     yylval.val.type = ConstBool;
	     yylval.val.Bool = 0;
	     return(NUMBER);
	   }
"break"	   return(BREAKSYM);
"return"   return(RETURNSYM);
"main"	   return(MAINSYM);
"if"	   return(IFSYM);
"else"	   return(ELSESYM);
"while"	   return(WHILESYM);
"for"	   return(FORSYM);
"read"	   return(READSYM);
"write"	   return(WRITESYM);
"sizeof"	   {yylval.ival = OP_SIZEOF; return(SIZEOF);}
{identifier}	   {
	     yylval.name = (char*)malloc(yyleng+1);
             strncpy(yylval.name, yytext, yyleng);
             yylval.name[yyleng]='\0';
//		printf("ID:'%s',Length:%d\n",yytext,yyleng);
	     Symbol sym = lookup(symtab, yylval.name);
	     if (sym==NULL) return(UNREGID);
/*	     sym = lookup(symtab, "factor");
		if (sym==NULL) printf("'factor' is not regeisted!\n");
	     	else printf("ID '%s' kind is %d\n",sym->name,sym->kind);
	     sym = lookup(symtab, yylval.name);*/
/*	     yylval.sym = sym;*/
	     switch (sym->kind)
	     {
		case OBJ_type: return(TYPEID);
		case OBJ_var: return(VARID);
		case OBJ_function: return(FUNCTIONID);
	     }
	   }
[0-9]+[eE]["+"|"-"]?[0-9]+	   {
	     yylval.val.type = ConstFloat;
	     yylval.val.Float = (float)atof(yytext);
	     return(NUMBER);
	   }
[0-9]+"."[0-9]+	   {
	     yylval.val.type = ConstFloat;
	     yylval.val.Float = (float)atof(yytext);
//	     printf("Float %f dectectived \n",yylval.val.Float);
	     return(NUMBER);
	   }
{character}	   {
	     yylval.val.type = ConstChar;
	     yylval.val.Char = yytext[1];
	     if (yytext[1]=='\\') {
		switch (yytext[2])
		{
		case 'v':
			yylval.val.Char = '\v';break;
		case 'f':
			yylval.val.Char = '\f';break;
		case 'r':
			yylval.val.Char = '\r';break;
		case 't':
			yylval.val.Char = '\t';break;
		case 'n':
	     		yylval.val.Char = '\n';break;
		case 'b':
	     		yylval.val.Char = '\b';break;
		default :
			break;
		}
	     }
	     return(NUMBER);
	   }
{bad_chr}	   {
	     printf("Error : Expect for ' .");
	     yylval.val.type = ConstChar;
	     yylval.val.Char = yytext[1];
	     return(NUMBER);
	   }
{string}	   {
	     int n=strlen(yytext)-2;
	     char *buffer=(char *)(StaticMemery+smp);
	     int i,m=0;
	     for (i=1;i<=n;i++)
	     {
                if (yytext[i]!='\\') buffer[m++]=yytext[i];
                else 
                {
                        i++;
		        switch (yytext[i])
		        {
		        case 'v':
			        buffer[m++] = '\v';break;
		        case 'f':
			        buffer[m++] = '\f';break;
		        case 'r':
			        buffer[m++] = '\r';break;
		        case 't':
			        buffer[m++] = '\t';break;
		        case 'n':
	             		buffer[m++] = '\n';break;
		        case 'b':
	             		buffer[m++] = '\b';break;
		        default :
			        break;
		        }
                }
	     }//The cpmlpex string support (allowed character like \n)
//	     strncpy(buffer,yytext+1,n);
	     buffer[m]='\0';
	     yylval.val.type = ConstString;
	     yylval.val.Int = smp;
	     smp+=m/4+1;
//	     smp+=n+1;
//	     if (smp%4) smp+=(4-smp%4);  //for the alline
//	     printf("Warning : String supporting is not finished yet >_<, use this may cause some unwanted result.\n");
	     return(NUMBER);
	   }
{bad_string}	   {
	     yylval.val.type = ConstString;
	     yylval.val.Int = (int)"";
	     printf("Error : String with out "" end.\n");
//	     printf("Warning : String supporting is not finished yet >_<, use this may cause some unwanted result.\n");
	     return(NUMBER);
	   }
0[0-7]+	   {
	     if (strlen(yytext)>NUMAXLEN*5/4+1) printf("Error : 31\n");
//	     yylval.ival = strtol(yytext,NULL,8);
	     yylval.val.type = ConstInt;
	     yylval.val.Int = strtol(yytext,NULL,8);
             return(NUMBER);
           }
0[xX][0-9A-Fa-f]+	   {
	     if (strlen(yytext)>NUMAXLEN*5/8+2) printf("Error : 31\n");
//	     yylval.ival = strtol(yytext,NULL,16);
	     yylval.val.type = ConstInt;
	     yylval.val.Int = strtol(yytext,NULL,16);
             return(NUMBER);
           }
[0-9]+	   {
	     if (strlen(yytext)>NUMAXLEN) printf("Error : 31\n");
//	     yylval.ival = strtol(yytext,NULL,10);
	     yylval.val.type = ConstInt;
	     yylval.val.Int = strtol(yytext,NULL,10);
             return(NUMBER);
           }
"<<"	   {yylval.ival = OP_LSH; return(LSH);}
">>"	   {yylval.ival = OP_RSH; return(RSH);}
"=="	   {yylval.ival = OP_EQL; return(EQL);}
"<="	   {yylval.ival = OP_LEQ; return(LEQ);}
"!="	   {yylval.ival = OP_NEQ; return(NEQ);}
"<"	   {yylval.ival = OP_LSS; return(LSS);}
">="	   {yylval.ival = OP_GEQ; return(GEQ);}
">"	   {yylval.ival = OP_GTR; return(GTR);}
"*"	   {yylval.ival = OP_MULT; return(MULT);}
"/"	   {yylval.ival = OP_DIV; return(DIV);}
"+"	   {yylval.ival = OP_PLUS; return(PLUS);}
"-"	   {yylval.ival = OP_MINUS; return(MINUS);}
"%"	   {yylval.ival = OP_MOD; return(MOD);}
"="	   {yylval.ival = OP_ASGN; return(ASGN);}
"&&"	   {yylval.ival = OP_ANDAND; return(ANDAND);}
"||"	   {yylval.ival = OP_OROR; return(OROR);}
"!"	   {yylval.ival = OP_NOT; return(NOT);}
"~"	   {yylval.ival = OP_BITNOT; return(BITNOT);}
"&"	   {yylval.ival = OP_AND; return(AND);}
"|"	   {yylval.ival = OP_OR; return(OR);}
"^"	   {yylval.ival = OP_XOR; return(XOR);}
","	   {return(COMMA);}
";"	   {return(SEMICOLON);}
"("	   {return(LPAREN);}
")"	   {return(RPAREN);}
[\n]	   {
//	     yyrow++;
	     yycolumn = 1;
//	     return(EOL);
	   }
.          {
	     printf("%s Ilegle Character\n",yytext);
	     return(EOL);
	   }

%%
