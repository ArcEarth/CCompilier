#pragma once
#include "pcode.h"

namespace C1
{
	namespace PCode
	{

		class Interpreter
		{
		public:
			void SetDebugLevel(int level);

			void Interpret(const CodeDome& Codedome);

		protected:
			static const size_t stacksize = 600;
			word SS[stacksize];	// datastore
			word DS[stacksize];

			long IR, BP, SP;		// program-, base-, topstack-registers

			bool debug = false, debug2 = false;

			long Locate(long base, long segment, long offset);

		};
	}
}