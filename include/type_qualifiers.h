#pragma once

namespace C1
{
	enum StorageClassSpecifierEnum
	{
		SCS_EXTERN,
		SCS_STATIC,
		SCS_AUTO,
		SCS_REGISTER,
		SCS_NONE,
	};

	enum TypeQualifierEnum
	{
		CONST = 0x1,
		RESTRICT = 0x2,
		VOLATILE = 0x4,
	};

	enum RecordKeywordEnum
	{
		STRUCT = 1,
		UNION = 2,
		ENUM = 3,
		CLASS = 4,
	};

	enum AccessibilityEnum
	{
		PRIVATE = 1,
		PROTECTED = 2,
		PUBLIC = 3,
	};

	struct Qualified
	{
		Qualified()
			: m_qulifier_mask(0)
		{}

		explicit Qualified(int mask)
			: m_qulifier_mask(mask)
		{}

		inline Qualified& operator=(int mask)
		{
			m_qulifier_mask = mask;
		}

		int Qualifiers() const
		{
			return m_qulifier_mask;
		}

		bool IsConst() const
		{
			return (m_qulifier_mask & TypeQualifierEnum::CONST) != 0;
		}
		bool IsRestrict() const
		{
			return (m_qulifier_mask & TypeQualifierEnum::RESTRICT) != 0;
		}
		bool IsVolatile() const
		{
			return (m_qulifier_mask & TypeQualifierEnum::VOLATILE) != 0;
		}
		int AddConst()
		{
			return m_qulifier_mask |= TypeQualifierEnum::CONST;
		}

		int RemoveConst()
		{
			return m_qulifier_mask &= ~TypeQualifierEnum::CONST;
		}

		int AddQualifiers(int qualfiers_mask)
		{
			return m_qulifier_mask |= qualfiers_mask;
		}

		int RemoveQualifiers(int qualfiers_mask)
		{
			return m_qulifier_mask &= ~qualfiers_mask;
		}

	protected:
		int m_qulifier_mask;
	};
#ifdef _IOSTREAM_
	inline std::ostream& operator<<(std::ostream& os, StorageClassSpecifierEnum scs)
	{
		switch (scs)
		{
		case 	SCS_EXTERN :
			os << "extern ";
			break;
		case	SCS_STATIC:
			os << "static ";
			break;
		case	SCS_AUTO:
			os << "auto ";
			break;
		case	SCS_REGISTER:
			os << "register ";
			break;
		case	SCS_NONE:
		default:
			break;
		}
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, TypeQualifierEnum tq)
	{
		switch (tq)
		{
		case 	CONST :
			os << "const ";
		case	RESTRICT:
			os << "restrict ";
		case	VOLATILE:
			os << "volatile ";
		default:
			break;
		}
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, RecordKeywordEnum rk)
	{
		switch (rk)
		{
		case 	STRUCT:
			os << "struct ";
		case	UNION:
			os << "union ";
		case	ENUM:
			os << "enum ";
		default:
			break;
		}
		return os;
	}
#endif

}