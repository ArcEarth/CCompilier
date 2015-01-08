#pragma once
#ifndef _ERROR_H
#define _ERROR_H

#include "../src/location.hh"
#include <string>
#include <vector>

namespace C1
{
	namespace Diagnostics
	{
		enum SeverityEnum
		{
			NOTE,
			WARNING,
			ERROR,
		};

		enum DiagnosticsEnum{
			#define errxx(a, b) a,
			#include "errcfg.h"
			#undef errxx
			LASTERR
		};

		class Message
		{
			const location& SourceLocation() const;
			SeverityEnum Severity() const;
			DiagnosticsEnum Kind() const;
			const std::string& Output() const;
		};

		class MessageContext
		{
			const Message* NewDiagMsg(const location& source_location, DiagnosticsEnum diag_kind, ...);
			std::vector<Message> m_Messages;
		};

	}



}
//// An error/warning message
//typedef struct errmsg{
//	bool isWarn;
//	int type;
//	char* msg;
//	int line;
//	int column;
//} *Errmsg;
//
//// Error factory
//typedef struct errfactory {
//	List	errors;
//	List	warnings;
//} *ErrFactory;
//
//// Function declarations on error message management
//Errmsg	newError(ErrFactory errfactory, int type, int line, int col);
//Errmsg	newWarning(ErrFactory errfactory, int type, int line, int col);
//void	dumpErrmsg(Errmsg error);
//ErrFactory newErrFactory();
//void	dumpErrors(ErrFactory errfactory);
//void	dumpWarnings(ErrFactory errfactory);
//void	destroyErrFactory(ErrFactory *errfact);

#endif // !_ERROR_H
