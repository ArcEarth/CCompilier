#pragma once
#include "cscanner.h"
#include "error.h"
#include "ast.h"

namespace C1 {
	class BisonParser;

	class Parser {
	public:
		Parser(std::istream& input, const std::string& file_name)
			: scanner(&input, nullptr, &ast_context), parser(ast_context, scanner)
		{
			ast_context.FileName = file_name;
			ast_context.SourceFile = &input;
			ast_context.type_context = new AST::TypeContext();
		}

		AST::TranslationUnit* parse() {
			std::ofstream errs(ast_context.FileName + ".log");
			parser.set_debug_level(1);
			parser.set_debug_stream(errs);
			parser.parse();
			return ast_context.CurrentTranslationUnit;
		}

	private:
		Diagnostics::MessageContext diag_context;
		AST::ASTContext ast_context;
		FlexScanner scanner;
		BisonParser parser;
	};
}
