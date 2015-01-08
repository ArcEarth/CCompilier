#pragma once

#include "c1.tab.hpp"
#include "ast_context.h"

// Only include FlexLexer.h if it hasn't been already included
#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

// Override the interface for yylex since we namespaced it
#undef YY_DECL
#define YY_DECL int C1::FlexScanner::yylex()
//#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
//	yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
//	yycolumn += yyleng;
// Include Bison for types / tokens


namespace C1 {
	class FlexScanner : public yyFlexLexer {
	public:
		typedef C1::BisonParser::symbol_type symbol_type;
		typedef C1::BisonParser::token_type token_type;
		typedef C1::BisonParser::location_type location_type;
		// save the pointer to yylval so we can change it, and invoke scanner

		FlexScanner(std::istream* pInput, std::ostream* pOutput = nullptr , AST::ASTContext* pContext = nullptr)
			: yyFlexLexer(pInput, pOutput)
		{
			this->pContext = pContext;
		}

		inline symbol_type NextToken()
		{
			current_symbol.~basic_symbol();
			yylex();
			return current_symbol;
		}

		void SetASTContext(const AST::ASTContext* context)
		{
			this->pContext = context;
		}

		const AST::ASTContext* GetASTContext() const
		{
			return pContext;
		}

	private:
		// Scanning function created by Flex; make this private to force usage
		// of the overloaded method so we can get a pointer to Bison's yylval
		int yylex();

		// for access the current building AST's information
		// more specifically , the current DeclContext for classify identifiers
		const AST::ASTContext* pContext;
		std::string	yycomment;
		symbol_type current_symbol;
		location_type loc;
		unsigned int yycolumn;
	};
}

