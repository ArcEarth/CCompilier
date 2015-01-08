#pragma once
#include <iostream>
#include "ast.h"
#include "cparser.h"
#include "interpreter.h"

namespace C1
{
	class ConsoleDriver
	{
		ConsoleDriver(std::istream& input, std::ostream&)
		: parser(input,"lalala.c")
		{

		}

	protected:
		Parser parser;
	};
}