#pragma once
#ifndef _TYPE_H_
#define _TYPE_H_

#include <list>
#include <string>
#include "decl_context.h"
#include "type_qualifiers.h"

namespace C1
{
	namespace AST
	{
		// Forward declarations
		class DeclContext;
		class Declaration;
		class Declarator;
		class TypeContext;

		// Represent an type in the program , it's an Semantic entity
		class Type
		{
		public:
			// register all the sub classes here
			enum TypeKindEnum
			{
				Unknown,
				Error,
				Basic,
				Void,
				Boolean,
				Character,
				Integer,
				Float,
				String,
				Pointer,
				Array,
				Function,
				Struct,
				Union,
				Enum,
				InitializerList,
				Typedef,
			};
		protected:
			TypeKindEnum m_Kind;

		public:
			TypeKindEnum Kind() const
			{ return m_Kind; }
			virtual bool IsPointerType() const;
			// is a pointer or array 
			virtual bool IsAddressType() const;
			virtual bool IsArrayType() const;
			// is a void / integer / float / error_type
			virtual bool IsBasicType() const;
			virtual bool IsIntegerType() const;
			virtual bool IsFloatType() const;
			virtual bool IsFunctionType() const;
			virtual bool IsStructType() const;
			virtual bool IsAliasType() const;
			virtual bool IsInitializerListType() const;
			virtual bool IsArithmeticType() const;
			virtual bool IsErrorType() const;
			//virtual bool IsStringType() const;
			virtual std::string ToString() const;
			virtual size_t Size() const = 0;
			virtual size_t Alignment() const = 0;
			virtual bool Match(const Type* type) const = 0;

			virtual ~Type();

			TypeContext* AffiliatedContext();
			const TypeContext* AffiliatedContext() const;
			void SetAffiliatedContext(TypeContext* val);

		protected:
			Type(TypeKindEnum kind);

			Type();

			TypeContext* m_AffiliatedContext;
		};

		//enum TypeMatchResult
		//{
		//	Matched = 0,
		//	ContextDismatch,
		//	QualifierDismatch,
		//	StructureDismatch,
		//	RecordNameDismatch,
		//};

		// Serve as an pointer of "Type" class, but provide qualifier information
		class QualType
		{
		public:
			QualType(Type *pType = nullptr, int qualifiers_mask = 0)
				: m_type(pType), m_qulifier_mask(qualifiers_mask)
			{
			}

			//QualType()
			//	: m_type(nullptr), m_qulifier_mask(0)
			//{}

			int Qualifiers() const
			{
				return m_qulifier_mask;
			}

			void reset(Type* type)
			{
				m_type = type;
			}

			Type* get()
			{
				return m_type;
			}

			const Type* get() const
			{
				return m_type;
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

			inline const Type &operator*() const{
				return *m_type;
			}

			inline const Type *operator->() const{
				return m_type;
			}

			inline Type &operator*() {
				return *m_type;
			}

			inline Type *operator->() {
				return m_type;
			}

			template<typename TargetType>
			inline TargetType* As()
			{
				return dynamic_cast<TargetType*>(m_type);
			}

			template<typename TargetType>
			inline const TargetType* As() const
			{
				return dynamic_cast<const TargetType*>(m_type);
			}

		public:
			void Dump(std::ostream& os);
			static const std::string& QulifierMaskToString(unsigned int qulifier_mask);

		private:
			Type* m_type;
			int m_qulifier_mask;
		};

		inline std::ostream& operator <<(std::ostream& os,QualType qual_type)
		{
			qual_type.Dump(os);
			return os;
		}

		// Remove the alias type in this type (not recursively)
		QualType remove_alias(QualType lhs);
		//bool type_match(const Type &lhs, const Type &rhs);
		bool type_match(QualType lhs, QualType rhs);
		inline bool operator==(const QualType &lhs, const QualType &rhs)
		{
			return type_match(lhs, rhs);
		}
		inline bool operator!=(const QualType &lhs, const QualType &rhs)
		{
			return !type_match(lhs, rhs);
		}
		bool is_type_assignable(QualType lhs, QualType rhs);

		inline bool operator<=(const QualType &lhs, const QualType &rhs)
		{
			return is_type_assignable(lhs, rhs);
		}

		inline bool operator>=(const QualType &lhs, const QualType &rhs)
		{
			return is_type_assignable(rhs, lhs);
		}

		// return nullptr if one of the operand is not arithmetic type
		QualType get_most_generic_arithmetic_type(QualType lhs, QualType rhs);
		QualType get_most_generic_float_type(QualType lhs, QualType rhs);
		QualType get_most_generic_integer_type(QualType lhs, QualType rhs);

		inline
		QualType MakeConst(Type* type)
		{
			return QualType(type, TypeQualifierEnum::CONST);
		}

		class InitializerListType : public Type
		{
		public:
			static const TypeKindEnum class_kind = InitializerList;
			InitializerListType(TypeContext* context);
			std::list<QualType>& ElementTypes() { return m_ElementTypes; }
			const std::list<QualType>& ElementTypes() const { return m_ElementTypes; }
			~InitializerListType();
			virtual size_t Size() const;

			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

			// Overrided for checking if all the type inside is uniform
			virtual bool IsArrayType() const;

		protected:
			std::list<QualType> m_ElementTypes;
		};
		
		class PointerType;

		class DereferencableType : public Type
		{
		public:
			QualType Base();
			const QualType Base() const;
			~DereferencableType();
			// Return the pointer of this dereferencers type
			PointerType*	GetPointerType();
		protected:
			DereferencableType(TypeKindEnum kind, QualType base);
			DereferencableType(QualType base);
			DereferencableType();
			QualType m_base;
		};

		class PointerType : public DereferencableType
		{
		public:
			static const TypeKindEnum class_kind = Pointer;
			PointerType(QualType base);
			~PointerType();
			virtual size_t Size() const;

			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

			virtual std::string ToString() const;

		};

		class ArrayType : public DereferencableType
		{
		public:
			static const TypeKindEnum class_kind = Array;
			static const size_t ArrayLengthAuto = -1;
			ArrayType(QualType base, size_t size);
			~ArrayType();
			// Return the number count of this array
			int Length() const { return m_size; }
			// Only should be called when deducing array size by initializer list
			// Only works when length is set to : ArrayType::ArrayLengthAuto
			void SetLength(size_t val) 
			{ if (m_size== ArrayLengthAuto) m_size = val; }
			
			virtual size_t Size() const;

			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

			virtual std::string ToString() const;

		protected:
			size_t m_size;
		};

		class RecordType : public Type
		{
		public:
			~RecordType();
			static const TypeKindEnum class_kind = Unknown;
			RecordType(TypeKindEnum kind, const std::string &name, DeclContext* define = nullptr);
			
			const std::string& Name() const { return m_Name; }
			bool IsCompleted() const { return m_Members != nullptr; }

			// We don't discard all the member's name detail here because we need them
			DeclContext* Members() { return m_Members; }
			const DeclContext* Members() const { return m_Members; }
			DeclContext* Definition() { return m_Members; }
			const DeclContext* Definition() const { return m_Members; }

			void SetDefinition(DeclContext* define) { m_Members = define; }

			static const char AnonymousName[];
		protected:
			std::string m_Name;
			DeclContext* m_Members;
		};

		class StructType : public RecordType
		{
		public:
			~StructType();
			static const TypeKindEnum class_kind = Struct;
			StructType(const std::string &name = "%anonymous", DeclContext* define = nullptr);

			virtual size_t Size() const;

			// This maybe not right... 
			// should it be the "minimum public quality" of all the member's alignment?
			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

			virtual std::string ToString() const;

		};

		class FunctionType : public Type
		{
		public:
			~FunctionType();
			static const TypeKindEnum class_kind = Function;
			FunctionType(QualType return_type, const std::list<QualType>& parameters);
			FunctionType(QualType return_type, std::list<QualType>&& parameters);
			QualType ReturnType() { return m_ReturnType; }
			const QualType ReturnType() const { return m_ReturnType; }
			// Since it's an type, we discard all the parameter's name
			std::list<QualType>& Parameters() { return m_Parameters; }
			const std::list<QualType>& Parameters() const { return m_Parameters; }

			virtual size_t Size() const;

			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

		private:
			QualType m_ReturnType;
			std::list<QualType> m_Parameters;
		};

		class EnumType : public RecordType
		{
			static const TypeKindEnum class_kind = Enum;
		};

		class UnionType : public RecordType
		{
			static const TypeKindEnum class_kind = Union;
		};

		class BasicType : public Type
		{
		public:
			static const TypeKindEnum class_kind = Basic;
			typedef Type::TypeKindEnum TypeKindEnum;
			~BasicType();
			size_t Size() const {
				return m_Size;
			}

			size_t Alignment() const {
				return m_Alignment;
			}

			virtual bool Match(const Type* type) const;

		protected:
			BasicType(TypeKindEnum kind, size_t size, size_t alignment);
			BasicType(TypeKindEnum kind, size_t size);


			size_t m_Size;
			size_t m_Alignment;
		};
		class ErrorType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Error;
			ErrorType();

			virtual std::string ToString() const;
			// Error type is always non-matchable
			virtual bool Match(const Type* type) const;

		};
		class VoidType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Void;
			VoidType();

			virtual std::string ToString() const;

		};
		class BooleanType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Integer;
			BooleanType();

			virtual std::string ToString() const;

		};
		class IntegerType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Integer;
			//IntegerType();
			IntegerType(size_t size);

			virtual std::string ToString() const;

		};
		class CharacterType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Character;
			CharacterType();

			virtual std::string ToString() const;

		};
		class FloatType : public BasicType
		{
		public:
			static const TypeKindEnum class_kind = Float;
			FloatType(size_t size);

			virtual std::string ToString() const;

		};
		// Represent a typedef type
		class AliasType : public Type
		{
		public:
			static const TypeKindEnum class_kind = Typedef;
			~AliasType();
			AliasType(QualType aliasd_type, const std::string& name);
			QualType Base() { return m_Base; }
			const QualType Base() const { return m_Base; }
			const std::string& Name() const { return m_Name; }
			virtual bool IsPointerType() const
			{ return m_Base->IsPointerType(); }
			virtual bool IsArrayType() const
			{ return m_Base->IsArrayType(); }
			virtual bool IsBasicType() const
			{ return m_Base->IsBasicType(); }
			virtual bool IsFunctionType() const
			{ return m_Base->IsFunctionType(); }
			virtual bool IsStructType() const
			{ return m_Base->IsStructType(); }
			virtual bool IsAliasType() const
			{ return true; }

			virtual size_t Size() const;

			virtual size_t Alignment() const;

			virtual bool Match(const Type* type) const;

			virtual std::string ToString() const;

		protected:
			QualType m_Base;
			std::string m_Name;
		};

		//StructType*	MakeStruct(const std::string& name, std::list<Declaration*>* field_decl_list);
		//StructType*	MakeStruct(std::list<Declaration*>* field_decl_list);
		//StructType*	MakeStruct(const std::string& name);
		//FunctionType* MakeFunction(QualType return_type, const std::list<QualType>& parameter_type_list);
		//FunctionType* MakeFunction(QualType return_type, std::list<QualType>&& parameter_type_list);
		//EnumType*	MakeEnum(const std::string& name, std::list<Declarator*>* enumerator_list);
		//EnumType*	MakeEnum(std::list<Declarator*>* enumerator_list);
		//EnumType*	MakeEnum(const std::string& name);
		//UnionType*	MakeUnionType();

		class TypeContext
		{
		public:
			TypeContext();

			//QualType	NewTypeFromDeclarator(QualType base_type,Declarator* declarator);
			//const Type*			NewTypeFromDeclarator();

			PointerType*NewPointerType(QualType base);
			ArrayType*	NewArrayType(QualType base, size_t size);
			//StructType*	NewStructType(const std::string& name, std::list<Declaration*>* field_decl_list);
			//StructType*	NewStructType(std::list<Declaration*>* field_decl_list);
			StructType*	NewStructType(const std::string& name);
			RecordType* NewRecordType(RecordKeywordEnum record_type, const std::string& name = "%anonymous");
			//FunctionType* NewFunctionType(QualType return_type,const std::list<QualType>& parameter_type_list);
			FunctionType* NewFunctionType(QualType return_type, std::list<QualType>&& parameter_type_list);
			//EnumType*	NewEnumType(const std::string& name, std::list<Declarator*>* enumerator_list);
			//EnumType*	NewEnumType(std::list<Declarator*>* enumerator_list);
			//EnumType*	NewEnumType(const std::string& name);
			//UnionType*	NewUnionType();
			AliasType*	NewAliasType(QualType base, const std::string& alias);
			InitializerListType* NewInitializerListType();

			const IntegerType*	Char() const { return m_Char.get(); }
			const IntegerType*	Short() const { return m_Short.get(); }
			const IntegerType*	Int() const { return m_Int.get(); }
			const IntegerType*	Long() const { return m_Long.get(); }
			const IntegerType*	UnsignedChar() const { return m_UnsignedChar.get(); }
			const IntegerType*	UnsignedShort() const { return m_UnsignedShort.get(); }
			const IntegerType*	UnsignedInt() const { return m_UnsignedInt.get(); }
			const BooleanType*	Bool() const { return m_Bool.get(); }
			const VoidType*		Void() const { return m_Void.get(); }
			const FloatType*	Float() const { return m_Float.get(); }
			const FloatType*	Double() const { return m_Double.get(); }
			// The alias for char*
			const PointerType*	String() const { return m_String.get(); }
			const ErrorType*	Error() const { return m_Error.get(); }

			IntegerType*	Char() { return m_Char.get(); }
			IntegerType*	Short() { return m_Short.get(); }
			IntegerType*	Int() { return m_Int.get(); }
			IntegerType*	Long() { return m_Long.get(); }
			IntegerType*	UnsignedChar() { return m_UnsignedChar.get(); }
			IntegerType*	UnsignedShort() { return m_UnsignedShort.get(); }
			IntegerType*	UnsignedInt() { return m_UnsignedInt.get(); }
			BooleanType*	Bool() { return m_Bool.get(); }
			VoidType*		Void() { return m_Void.get(); }
			FloatType*		Float() { return m_Float.get(); }
			FloatType*		Double() { return m_Double.get(); }
			PointerType*	String() { return m_String.get(); }
			ErrorType*		Error() { return m_Error.get(); }

			size_t	AddressWidth() const { return m_AddressWidth; }
			size_t	AddressAlignment() const { return m_AddressAllignment; }
			//void	SetPointerSize(size_t val) { m_AddressWidth = val; }

			size_t	WordSize() const { return m_WordSize; }
			size_t	WordAllignment() const { return m_WordAlignment; }
			//void	SetIntSize(size_t val) { m_IntSize = val; }
		protected:
			std::unique_ptr<IntegerType>	m_Char;
			std::unique_ptr<IntegerType>	m_Short;
			std::unique_ptr<IntegerType>	m_Int;
			std::unique_ptr<IntegerType>	m_Long;
			std::unique_ptr<IntegerType>	m_UnsignedChar;
			std::unique_ptr<IntegerType>	m_UnsignedShort;
			std::unique_ptr<IntegerType>	m_UnsignedInt;
			std::unique_ptr<BooleanType>	m_Bool;
			std::unique_ptr<VoidType>		m_Void;
			std::unique_ptr<FloatType>		m_Float;
			std::unique_ptr<FloatType>		m_Double;
			std::unique_ptr<PointerType>	m_String;
			std::unique_ptr<ErrorType>		m_Error;

			size_t							m_AddressWidth;
			size_t							m_AddressAllignment;
			size_t							m_WordSize;
			size_t							m_WordAlignment;

			std::list<std::unique_ptr<Type>> m_ResourcesPool;
		};

		inline PointerType	*MakePointer(QualType base)
		{
			return base->AffiliatedContext()->NewPointerType(base);
		}

		inline ArrayType	*MakeArray(QualType base, size_t size)
		{
			return base->AffiliatedContext()->NewArrayType(base, size);
		}

		inline AliasType	*MakeAlias(QualType base, const std::string& alias)
		{
			return base->AffiliatedContext()->NewAliasType(base, alias);
		}
	}
}

#endif // if !_TYPE_H_