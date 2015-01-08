#pragma once

#ifndef _OPERATORS_H_
#define _OPERATORS_H_
namespace C1
{
	enum OperatorsEnum
	{
#define opxx(a,b,c) OP_##a,
#include "opcfg.h"
#undef opxx
	};

#ifdef _IOSTREAM_
	static char OperatorLiterals [][10] = {
#define opxx(a,b,c) b,
#include "opcfg.h"
#undef opxx
	};

	inline std::ostream& operator<<(std::ostream& os, OperatorsEnum op)
	{
		return os << OperatorLiterals[op];
	}

#endif

}

#endif // !_OPERATORS_H_
