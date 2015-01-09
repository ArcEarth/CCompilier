/*
 * expr.lex : Scanner for a simple
 *            expression parser.
 */
%top{
#include <cstdint>
}
%{
#include "cscanner.h"
//using namespace std;
using namespace C1;
using namespace C1::AST;
%}

%option noyywrap c++ yyclass="FlexScanner" yylineno

%x COMMENT

digit				[0-9]+
octal				0[0-7]+	
hex					0[Xx][0-9A-Fa-f]+
unsigned			{digit}[uU]
decimal				[+-]?({digit})?"."{digit}
sci_float			{decimal}([eE]([+-])?{digit})?
ident				[A-Za-z_][A-Za-z_0-9]*
line_comment		"//"[^\n]+
char_literal		"'"([^"'"\n])*"'"
string_literal		"\""([^"'"\n])*"\""
bad_char_literal	"\""([^"'"\n])*\n
bad_string_literal	"'"([^"'"\n])*\n


%{
#define YY_USER_ACTION loc.columns (yyleng);
%}

%{
typedef C1::BisonParser::token token;
%} 

%%
%{
	//evert time calls yylex()
	loc.step ();
%}
"/*"    		{
					BEGIN(COMMENT);
					yycomment = "";
				}
<COMMENT>"*/" 	{
					BEGIN(INITIAL);
		    	}
<COMMENT>[\n]	{
					loc.lines ();
					yycomment += yytext;
				}
<COMMENT>[^*/\n]+ {
					yycomment += yytext;
				}
<COMMENT>[*/] {
					yycomment += yytext;
				}
<COMMENT><<EOF>> {
				    printf("Error : File end during comments.\n");
				    current_symbol.move(C1::BisonParser::make_END(loc));
				    return token::END;	
				}
{line_comment} 	{
					loc.lines ();
					loc.step (); 
				}
<<EOF>> 		{
				    current_symbol.move(C1::BisonParser::make_END(loc));
				    return token::END;	
				}
[\n] 			{ 
					loc.lines ();
					loc.step (); 
				}
[\t ]*     		{ 
					loc.step (); 
				}

"auto"			{ current_symbol.move(C1::BisonParser::make_AUTO(SCS_AUTO,loc)); return token::AUTO;  }
"break"			{ current_symbol.move(C1::BisonParser::make_BREAK(loc)); return token::BREAK;  }
"case"			{ current_symbol.move(C1::BisonParser::make_CASE(loc)); return token::CASE;  }
"char"			{ current_symbol.move(C1::BisonParser::make_CHAR(loc)); return token::CHAR;  }
"continue"		{ current_symbol.move(C1::BisonParser::make_CONTINUE(loc)); return token::CONTINUE;  }
"default"		{ current_symbol.move(C1::BisonParser::make_DEFAULT(loc)); return token::DEFAULT;  }
"do"			{ current_symbol.move(C1::BisonParser::make_DO(loc)); return token::DO;  }
"double"		{ current_symbol.move(C1::BisonParser::make_DOUBLE(loc)); return token::DOUBLE;  }
"else"			{ current_symbol.move(C1::BisonParser::make_ELSE(loc)); return token::ELSE;  }
"enum"			{ current_symbol.move(C1::BisonParser::make_ENUM(ENUM,loc)); return token::ENUM;  }
"extern"		{ current_symbol.move(C1::BisonParser::make_EXTERN(SCS_EXTERN,loc)); return token::EXTERN;  }
"float"			{ current_symbol.move(C1::BisonParser::make_FLOAT(loc)); return token::FLOAT;  }
"for"			{ current_symbol.move(C1::BisonParser::make_FOR(loc)); return token::FOR;  }
"goto"			{ current_symbol.move(C1::BisonParser::make_GOTO(loc)); return token::GOTO;  }
"if"			{ current_symbol.move(C1::BisonParser::make_IF(loc)); return token::IF;  }
"int"			{ current_symbol.move(C1::BisonParser::make_INT(loc)); return token::INT;  }
"bool"			{ current_symbol.move(C1::BisonParser::make_INT(loc)); return token::BOOL;  }
"long"			{ current_symbol.move(C1::BisonParser::make_LONG(loc)); return token::LONG;  }
"read"			{ current_symbol.move(C1::BisonParser::make_READ(loc)); return token::READ;  }
"register"		{ current_symbol.move(C1::BisonParser::make_REGISTER(SCS_REGISTER,loc)); return token::REGISTER;  }
"return"		{ current_symbol.move(C1::BisonParser::make_RETURN(loc)); return token::RETURN;  }
"short"			{ current_symbol.move(C1::BisonParser::make_SHORT(loc)); return token::SHORT;  }
"signed"		{ current_symbol.move(C1::BisonParser::make_SIGNED(loc)); return token::SIGNED;  }
"sizeof"		{ current_symbol.move(C1::BisonParser::make_SIZEOF(OP_SIZEOF,loc)); return token::SIZEOF;  }
"static"		{ current_symbol.move(C1::BisonParser::make_STATIC(SCS_STATIC,loc)); return token::STATIC;  }
"struct"		{ current_symbol.move(C1::BisonParser::make_STRUCT(C1::STRUCT,loc)); return token::STRUCT;  }
"switch"		{ current_symbol.move(C1::BisonParser::make_SWITCH(loc)); return token::SWITCH;  }
"typedef"		{ current_symbol.move(C1::BisonParser::make_TYPEDEF(loc)); return token::TYPEDEF;  }
"union"			{ current_symbol.move(C1::BisonParser::make_UNION(UNION,loc)); return token::UNION;  }
"unsigned"		{ current_symbol.move(C1::BisonParser::make_UNSIGNED(loc)); return token::UNSIGNED;  }
"void"			{ current_symbol.move(C1::BisonParser::make_VOID(loc)); return token::VOID;  }
"while"			{ current_symbol.move(C1::BisonParser::make_WHILE(loc)); return token::WHILE;  }
"write"			{ current_symbol.move(C1::BisonParser::make_WRITE(loc)); return token::WRITE;  }
"restrict"		{ current_symbol.move(C1::BisonParser::make_RESTRICT(RESTRICT,loc)); return token::RESTRICT; }
"const"			{ current_symbol.move(C1::BisonParser::make_CONST(CONST,loc)); return token::CONST; }
"volatile"		{ current_symbol.move(C1::BisonParser::make_VOLATILE(VOLATILE,loc)); return token::VOLATILE; }
"true"			{
					Expr* node = new AST::BooleanLiteral(pContext->type_context,yytext,true);
					current_symbol.move(C1::BisonParser::make_BOOL_LITERAL(node,loc));
					return token::BOOL_LITERAL;
				}
"false"			{
					Expr* node = new AST::BooleanLiteral(pContext->type_context,yytext,false);
					current_symbol.move(C1::BisonParser::make_BOOL_LITERAL(node,loc));
					 return token::BOOL_LITERAL;
				}
{sci_float}		{
					auto val = strtof(yytext,NULL);
					Expr* node = new AST::FloatLiteral(pContext->type_context,yytext,val);
					current_symbol.move(C1::BisonParser::make_FLOAT_LITERAL(node,loc)); return token::INT_LITERAL; 
				}
{octal} 		{ 
					auto val = strtol(yytext,NULL,8);
					Expr* node = new AST::IntegerLiteral(pContext->type_context,yytext,val,8);
					current_symbol.move(C1::BisonParser::make_INT_LITERAL(node,loc)); return token::INT_LITERAL; 
				}
{digit}    		{ 
					auto val = strtol(yytext,NULL,10);
					Expr* node = new AST::IntegerLiteral(pContext->type_context,yytext,val,10);
					current_symbol.move(C1::BisonParser::make_INT_LITERAL(node,loc)); return token::INT_LITERAL; 
				}
{hex}		   	{ 
					auto val = strtol(yytext,NULL,16);
					Expr* node = new AST::IntegerLiteral(pContext->type_context,yytext,val,16);
					current_symbol.move(C1::BisonParser::make_INT_LITERAL(node,loc)); return token::INT_LITERAL; 
				}
{string_literal} 	{
					Expr* node = new AST::StringLiteral(pContext->type_context,yytext);
					current_symbol.move(C1::BisonParser::make_STRING_LITERAL(node,loc));
					return token::STRING_LITERAL; 
				}
{ident}	 		{ 
					std::string val(yytext);
					auto decl = pContext->current_context()->lookup(val);
					if (decl){
						if (dynamic_cast<const TypeDeclaration*>(decl)!=nullptr) {
							current_symbol.move(C1::BisonParser::make_TypeIdentifier(yytext,loc));
							return token::TypeIdentifier;
						} else {
							current_symbol.move(C1::BisonParser::make_ObjectIdentifier(yytext,loc));
							return token::ObjectIdentifier;
						}
					} else
					current_symbol.move(C1::BisonParser::make_NewIdentifier(yytext,loc));
					return token::NewIdentifier; 
				}
"="		{ current_symbol.move(C1::BisonParser::make_ASSIGN(OP_ASSIGN,loc)); return token::ASSIGN;  }
"<"		{ current_symbol.move(C1::BisonParser::make_LSS(OP_LSS,loc)); return token::LSS;  }
"=="	{ current_symbol.move(C1::BisonParser::make_EQL(OP_EQL,loc)); return token::EQL;  }
"<="	{ current_symbol.move(C1::BisonParser::make_LEQ(OP_LEQ,loc)); return token::LEQ;  }
"!="	{ current_symbol.move(C1::BisonParser::make_NEQ(OP_NEQ,loc)); return token::NEQ; }
">"		{ current_symbol.move(C1::BisonParser::make_GTR(OP_GTR,loc)); return token::GTR; }
">="	{ current_symbol.move(C1::BisonParser::make_GEQ(OP_GEQ,loc)); return token::GEQ; }						
"+"		{ current_symbol.move(C1::BisonParser::make_ADD(OP_ADD,loc)); return token::ADD;  }
"-"		{ current_symbol.move(C1::BisonParser::make_SUB(OP_SUB,loc)); return token::SUB;  }
"*"		{ current_symbol.move(C1::BisonParser::make_MUL(OP_MUL,loc)); return token::MUL;  }
"/"		{ current_symbol.move(C1::BisonParser::make_DIV(OP_DIV,loc)); return token::DIV;  }
"%"		{ current_symbol.move(C1::BisonParser::make_MOD(OP_MOD,loc)); return token::MOD;  }
"."		{ current_symbol.move(C1::BisonParser::make_DOT(OP_DOT,loc)); return token::DOT;  }
"->"	{ current_symbol.move(C1::BisonParser::make_ARROW(OP_ARROW,loc)); return token::ARROW;  }
"&&"	{ current_symbol.move(C1::BisonParser::make_ANDAND(OP_ANDAND,loc)); return token::ANDAND;  }
"||"	{ current_symbol.move(C1::BisonParser::make_OROR(OP_OROR,loc)); return token::OROR;  }
"&"		{ current_symbol.move(C1::BisonParser::make_AND(OP_AND,loc)); return token::AND;  }
"^"		{ current_symbol.move(C1::BisonParser::make_XOR(OP_XOR,loc)); return token::XOR;  }
"|"		{ current_symbol.move(C1::BisonParser::make_OR(OP_OR,loc)); return token::OR;  }
"<<"	{ current_symbol.move(C1::BisonParser::make_LSH(OP_LSH,loc)); return token::LSH;  }
">>"	{ current_symbol.move(C1::BisonParser::make_RSH(OP_RSH,loc)); return token::RSH;  }
"+=" 	{ current_symbol.move(C1::BisonParser::make_ADD_ASSIGN(OP_ADD_ASSIGN,loc)); return token::ADD_ASSIGN;  }
"-=" 	{ current_symbol.move(C1::BisonParser::make_SUB_ASSIGN(OP_SUB_ASSIGN,loc)); return token::SUB_ASSIGN;  }
"*=" 	{ current_symbol.move(C1::BisonParser::make_MUL_ASSIGN(OP_MUL_ASSIGN,loc)); return token::MUL_ASSIGN;  }
"/=" 	{ current_symbol.move(C1::BisonParser::make_DIV_ASSIGN(OP_DIV_ASSIGN,loc)); return token::DIV_ASSIGN;  }
"%=" 	{ current_symbol.move(C1::BisonParser::make_MOD_ASSIGN(OP_MOD_ASSIGN,loc)); return token::MOD_ASSIGN;  }
"&=" 	{ current_symbol.move(C1::BisonParser::make_AND_ASSIGN(OP_AND_ASSIGN,loc)); return token::AND_ASSIGN;  }
"|=" 	{ current_symbol.move(C1::BisonParser::make_OR_ASSIGN(OP_OR_ASSIGN,loc)); return token::OR_ASSIGN;  }
"^=" 	{ current_symbol.move(C1::BisonParser::make_XOR_ASSIGN(OP_XOR_ASSIGN,loc)); return token::XOR_ASSIGN;  }
"<<=" 	{ current_symbol.move(C1::BisonParser::make_LSH_ASSIGN(OP_LSH_ASSIGN,loc)); return token::LSH_ASSIGN;  } 
">>=" 	{ current_symbol.move(C1::BisonParser::make_RSH_ASSIGN(OP_RSH_ASSIGN,loc)); return token::RSH_ASSIGN;  }
"++" 	{ current_symbol.move(C1::BisonParser::make_ADDADD(OP_ADDADD,loc)); return token::ADDADD;  }
"--" 	{ current_symbol.move(C1::BisonParser::make_SUBSUB(OP_SUBSUB,loc)); return token::SUBSUB;  }

"{"		{ current_symbol.move(C1::BisonParser::make_LBRACE(loc)); return token::LBRACE;  }
"}"		{ current_symbol.move(C1::BisonParser::make_RBRACE(loc)); return token::RBRACE;  }
"("		{ current_symbol.move(C1::BisonParser::make_LPAREN(loc)); return token::LPAREN;  }
")"		{ current_symbol.move(C1::BisonParser::make_RPAREN(loc)); return token::RPAREN;  }
"["		{ current_symbol.move(C1::BisonParser::make_LBRACKET(loc)); return token::LBRACKET;  }
"]"		{ current_symbol.move(C1::BisonParser::make_RBRACKET(loc)); return token::RBRACKET;  }
","		{ current_symbol.move(C1::BisonParser::make_COMMA(loc)); return token::COMMA;  }
":"		{ current_symbol.move(C1::BisonParser::make_COLON(loc)); return token::COLON;  }
";"		{ current_symbol.move(C1::BisonParser::make_SEMICOLON(loc)); return token::SEMICOLON;  }

.		{ 
			loc.step();
			printf("Invaliad char.");
		}

%%
