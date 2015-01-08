#include "stdafx.h"
#include <fstream>
#include "driver.h"
using namespace std;
using namespace C1;

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