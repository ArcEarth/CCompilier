#include "stdafx.h"
#include <fstream>
#include "driver.h"
using namespace std;
using namespace C1;

class Translator
{
	ostream& os;
	void translate(AST::Node* node)
	{
		using namespace AST;
		if (node->Is<TranslationUnit>())
		{
			auto unit = node->As<TranslationUnit>();
			for (const auto& decl : unit->Children())
			{ 
				if (decl->Is<FunctionDeclaration>())
				{ 
					auto func = dynamic_cast<FunctionDeclaration*>(decl);
					auto name = func->Name();
					if (isalpha(name.front()))
					{
						name.front() = toupper(name.front());
					}
					func->Rename(name);
				}
			}
		}
		else if (node->Is<TypedefNameSpecifier>())
		{
			auto typespec = node->As<TypeSpecifier>();
			auto type = typespec->RepresentType();
			if (type->IsAliasType())
			{
				auto alias = dynamic_cast<AliasType*>(type);
				alias->Name();
			}
		}

		auto alias = dynamic_cast<AST::AliasType*>(node);
		if (alias == nullptr)
		{
			node->Dump(os);
		}
		else
		{

		}
	}
};

int main(int argc, char *argv [])
{
	if (argc < 2)
	{
		cout << "missing command parameter : [source file(s)]" << endl;
		return 1;
	}
	string sourceFileName(argv[1]);
	ifstream source(sourceFileName);
	//ConsoleDriver driver(&source, &cout);
	C1::Parser parser(source, sourceFileName);
	auto ast = parser.parse();
	cout << endl << "Dump source file from AST:" << endl;

	cout << *ast;

	C1::PCode::CodeDome codes;
	codes << *ast;

	using C1::PCode::operator<<;
	cout << codes;
	
	system("PAUSE");

	C1::PCode::Interpreter interpreter;
	if (argc >= 3 && argv[3] == "d")
		interpreter.SetDebugLevel(1);
	if (argc >= 3 && argv[3] == "d2")
		interpreter.SetDebugLevel(2);
	interpreter.Interpret(codes);
	//system("PAUSE");
}