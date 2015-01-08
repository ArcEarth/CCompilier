#pragma once
#ifndef _PCODE_H_
#define _PCODE_H_
#include "operators.h"
#include <vector>
namespace C1
{
	namespace PCode{
		typedef C1::OperatorsEnum OperatorsEnum;

		enum fct {
			lit, opr, lod, sto, cal, isp, jmp, jpc, jpe, lar, sar
		};

		static char fctname[][4] = { "lit", "opr", "lod", "sto", "cal", "isp", "jmp", "jpc", "jpe", "lar", "sar" };

		struct instruction{
			enum fct f;		// function code
			long l; 		// local/global
			long a; 		// displacement address
		};

#ifdef _IOSTREAM_
		inline std::ostream& operator<<(std::ostream& os, fct f)
		{
			return os << fctname[f];
		}

		inline std::ostream& operator<<(std::ostream& os,const instruction &ir)
		{
			if (ir.f == opr)
				os << ir.f << "\t" << ir.l << "\t" << OperatorsEnum(ir.a);
			else
				os << ir.f << "\t" <<ir.l << "\t" << ir.a;
			return os;
		}
#endif

		enum IOTypeEnum{
#define tpxx(a, b) TP_##a,
			tpxx(BOOL, "bool")
			tpxx(CHAR, "char")
			tpxx(INT, "int")
			tpxx(ADDRESS, "*")
			tpxx(FLOAT, "float")
			tpxx(STRING, "char *")
			TP_LAST
#undef tpxx
		};

		enum IndexBaseMethod
		{
			SS = 0, // Global in Stack Segment
			SS_BP = 1, // Stack Segment, Base Pointer
			DS = 2, // Data Segment
		};


		inline instruction gen(fct f, long l, long a)
		{
			return instruction{ f, l, a };
		}

		typedef union{
			long i;
			float f;
			char c[4];
		} word;

		// Represent 
		class CodeDome
		{
		public:
			CodeDome();
			// Static Data
			size_t EmplaceString(const std::string &str_data);
			size_t EmplaceFloat(float data);
			size_t EmplaceDataBlcok(size_t size_in_byte, char* data = nullptr);
			size_t EmplaceInteger(int data);
			
			size_t InstructionCount() const { return CodeSegment.size(); }

			// Return the address of this instruction
			size_t EmplaceInstruction(fct f, long l, long a);

			size_t PushInstruction(const instruction& ir);

			instruction& Instruction(size_t index) { return CodeSegment[index]; }
			const instruction& Instruction(size_t index) const { return CodeSegment[index]; }

			size_t						SP;
			static const size_t			CallStorageSize = 2;

			std::vector<instruction>& Codes() { return CodeSegment; }
			const std::vector<instruction>& Codes() const { return CodeSegment; }

			std::vector<word>& Datas() { return DataSegment; }
			const std::vector<word>& Datas() const { return DataSegment; }
		protected:
			std::vector<word>			DataSegment;
			std::vector<instruction>	CodeSegment;
		};

		inline CodeDome& operator<<(CodeDome& dome, const instruction& ir)
		{
			dome.PushInstruction(ir);
			return dome;
		}

#ifdef _IOSTREAM_

		inline std::ostream& operator<<(std::ostream& os, const CodeDome& dome)
		{
			os << "DS:" << std::endl;
			size_t addr = 0;
			for (auto word : dome.Datas())
			{
				os << word.c[0] << word.c[1] << word.c[2] << word.c[3];
			}
			os << std::endl << "CS:" << std::endl;
			addr = 0;
			for (auto ir : dome.Codes())
			{
				os << addr++ <<"\t" << ir << std::endl;
			}
			return os;
		}

#endif
	}
}

/*  lit 0, a : load constant a
	opr 0, a : execute operation a
	lod l, a : load variable l, a
	sto l, a : store variable l, a
	cal l, a : call procedure a at level l
	Int 0, a : increment t-register by a
	jmp 0, a : jump to a
	jpc 0, a : jump conditional to a       */

#define BLOCK_MARK_SIZE 3

#endif //if! _PCODE_H_
