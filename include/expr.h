#pragma once
#include <array>
#include <memory>
#include "ast_node.h"
#include "type.h"
#include "declaration.h"
#include "declarator.h"
#include "operators.h"
namespace C1
{
	namespace AST
	{
		enum ExprValueType
		{
			RValue,
			LValue,
			XValue,
		};

		union ComposedValue
		{
			bool Bool;
			char Char;
			int Int;
			float Float;
			const char* String;
			void* Pointer;
			std::vector<ComposedValue>* ValueList;
			//Well, array is a little tricky, we have initializer list but not this.
			//void* Array;
		};

		class Expr : public Node
		{
		public:
			typedef ComposedValue ComposedValue;
			virtual QualType ReturnType() const = 0;
			// Only valid for L-Value Expression , return -1
			virtual void GenerateLValue(C1::PCode::CodeDome& dome);
			virtual ExprValueType ValueType() const = 0;
			virtual bool HasSideEffect() const = 0;
			// Only valid for Evaluatable Expression , return 0
			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const = 0;
			//protected:
			//	value_type m_Value;
		};

		class CallExpr : public Expr
		{
		public:
			const std::list<Expr*>& Arguments() const
			{
				return m_Arguments;
			}

			const Expr* FunctionalExpr() const
			{
				return m_FuncExpr.get();
			}

			~CallExpr();

			static CallExpr* MakeCallExpr(Expr*, std::list<Expr*>*);

			CallExpr(Expr*, std::list<Expr*>*);

			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			// Always X-Value (pure R Value)
			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			// Always false, ignore the C++11 const-expr specifier
			virtual bool Evaluatable() const;

			virtual void Generate(C1::PCode::CodeDome& dome);


		private:
			std::unique_ptr<Expr> m_FuncExpr;
			// Call Expr owns all it's arguments
			std::list<Expr*> m_Arguments;
		};

		class DeclRefExpr : public Expr
		{
		public:
			static DeclRefExpr* MakeDeclRefExpr(DeclContext* lookup_context, TypeContext* type_context, const std::string &name);

			const ValueDeclaration* RefDecl() const { return m_RefDecl; }

			const std::string& Identifier() const { return m_Name; }
			// It's safe.
			void SetIdentifier(const std::string val);

			const DeclContext* RefDeclContext() const { return m_RefContext; }
			// It's safe.
			void SetRefDeclContext(DeclContext* reference_context);

			void Dump(std::ostream&) const;

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);
			virtual void Generate(C1::PCode::CodeDome& dome);

			ValueDeclaration* ResoloveReference();

		protected:
			DeclRefExpr(DeclContext* lookup_context, TypeContext* type_context, const std::string &name);

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual bool Evaluatable() const;

			virtual ComposedValue Evaluate() const;


			std::string m_Name;

			DeclContext*	  m_RefContext; //The declaration context which the where this ref should lookup
			TypeContext*	  m_TypeContext;
			// DeclRefExpr don't owning its reference declaration
			// It's a cache
			ValueDeclaration* m_RefDecl;
		};

		class ParenExpr : public Expr
		{
		public:
			ParenExpr(Expr* base);
			Expr* Base() { return m_Base.get(); }
			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual bool Evaluatable() const;

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual ComposedValue Evaluate() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Expr> m_Base;
		};

		// ++ -- + - ! ~
		class UnaryExpr : public Expr
		{
		public:
			static UnaryExpr* MakeUnaryExpr(OperatorsEnum op, Expr* sub_expr);

			const Expr* SubExpr() const { return m_SubExpr.get(); }
			Expr* SubExpr() { return m_SubExpr.get(); }
			OperatorsEnum Operator() const { return m_Operator; }
			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual bool Evaluatable() const;

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual ComposedValue Evaluate() const;

			virtual void Generate(C1::PCode::CodeDome& dome);
		protected:
			UnaryExpr(OperatorsEnum op, Expr* sub_expr);


			OperatorsEnum m_Operator;
			std::unique_ptr<Expr> m_SubExpr;
		};

		class PosfixExpr : public UnaryExpr
		{
		public:
			static PosfixExpr* MakePosfixExpr(OperatorsEnum op, Expr* Base);

			PosfixExpr(OperatorsEnum op, Expr* Base);
			void Dump(std::ostream&) const;

			virtual ExprValueType ValueType() const;

			//virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
		};

		class IncrementExpr : public UnaryExpr
		{
			// Always LValue
			virtual ExprValueType ValueType() const;
			// Always True
			virtual bool HasSideEffect() const;
			// Always False ,something like "++3" is always illegal
			virtual bool Evaluatable() const;

		};

		class PosfixIncrementExpr : public IncrementExpr
		{};

		class BinaryExpr : public Expr
		{
		public:
			BinaryExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);
			const Expr* LeftSubExpr() const { return m_SubExprs[0].get(); }
			const Expr* RightSubExpr() const { return m_SubExprs[1].get(); }
			Expr* LeftSubExpr() { return m_SubExprs[0].get(); }
			Expr* RightSubExpr() { return m_SubExprs[1].get(); }
			const OperatorsEnum Operator() const { return m_Operator; }
			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			// Always R-Value
			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

		protected:
			OperatorsEnum m_Operator;
			std::array<std::unique_ptr<Expr>, 2> m_SubExprs;
		};

		class DereferenceExpr : public UnaryExpr
		{
		public:
			static DereferenceExpr* MakeDereferenceExpr(Expr* base);
			// Always LValue
			virtual ExprValueType ValueType() const;
			// Always False
			virtual bool Evaluatable() const;
			// de-ref of the sub-expr
			virtual QualType ReturnType() const;
			// Side effect : Always False , if there is no operator overload
		protected:
			DereferenceExpr(Expr* base);

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class AddressOfExpr : public UnaryExpr
		{
		public:
			static AddressOfExpr* MakeAddressOfExpr(Expr* base);
			// Always RValue
			virtual ExprValueType ValueType() const;
			// Always False
			virtual bool Evaluatable() const;
			// const (pointer of the sub-expression's type)
			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			AddressOfExpr(Expr* base);


		};

		class AssignExpr : public BinaryExpr
		{
		public:
			static AssignExpr* MakeAssignExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);

			bool IsComposed() const { return Operator() != OP_ASSIGN; }
			// return OP_ASGN if is OP_ASGN itself
			const OperatorsEnum PrefixedOperator() const;

			static OperatorsEnum GetComposedOperator(OperatorsEnum op);
			// The lhs
			virtual QualType ReturnType() const;

			// always LValue
			virtual ExprValueType ValueType() const;

			// always true
			virtual bool HasSideEffect() const;

			// Same as rhs for "=" , always false for ComposedAssignExpr 
			virtual bool Evaluatable() const;

			virtual ComposedValue Evaluate() const;

			virtual void Dump(std::ostream& ostr) const;

		protected:
			AssignExpr(Expr* lhs, Expr* rhs);
			AssignExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class IndexExpr : public BinaryExpr
		{
		public:
			IndexExpr(Expr* host_expr, Expr* index_expr);
			static IndexExpr* MakeIndexExpr(Expr* host_expr, Expr* index_expr);
			Expr* Host() { return LeftSubExpr(); }
			Expr* Index() { return RightSubExpr(); }
			const Expr* Host() const { return LeftSubExpr(); }
			const Expr* Index() const { return RightSubExpr(); }
			void Dump(std::ostream&) const;

			// De-ref of lhs
			virtual QualType ReturnType() const;
			// Always LValue
			virtual ExprValueType ValueType() const;

			virtual ComposedValue Evaluate() const;

			// Always false
			virtual bool Evaluatable() const;

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual void Generate(C1::PCode::CodeDome& dome);

		};


		class ArithmeticExpr : public BinaryExpr
		{
		public:
			static ArithmeticExpr* MakeArithmeticExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);
		protected:
			ArithmeticExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class LogicExpr : public BinaryExpr
		{
		public:
			static LogicExpr* MakeLogicExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);
		protected:
			LogicExpr(OperatorsEnum op, Expr* lhs, Expr* rhs);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class ConditionalExpr : public Expr
		{
		public:
			ConditionalExpr(Expr* condition, Expr* true_expr, Expr* false_expr);
			void Dump(std::ostream&) const;

			const Expr* FalseExpr() const { return m_FalseExpr; }
			void SetFalseExpr(Expr* val) { m_FalseExpr = val; }
			const Expr* TrueExpr() const { return m_TrueExpr; }
			void SetTrueExpr(Expr* val) { m_TrueExpr = val; }
			const Expr* Condition() const { return m_Condition; }
			void SetCondition(Expr* val) { m_Condition = val; }

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

		protected:
			Expr* m_Condition, *m_TrueExpr, *m_FalseExpr;
		};

		class QualifiedTypeSpecifier;

		class TypeExpr : public Node
		{
		public:
			TypeExpr(QualifiedTypeSpecifier* qual_type_specifier, Declarator* declarator);
			QualifiedTypeSpecifier* DeclSpecifier() { return m_Specifier.get(); }
			void SetDeclSpecifier(QualifiedTypeSpecifier*);
			Declarator*	DeclDeclarator() { return m_Declarator.get(); }
			void SetDeclarator(Declarator*);
			QualType DeclType() const { return m_Type; }
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Declarator> m_Declarator;
			std::unique_ptr<QualifiedTypeSpecifier> m_Specifier;
			QualType	m_Type; // It cached the type defined by Specifier and Declarator
		};

		class SizeofExpr : public  UnaryExpr
		{
		public:
			SizeofExpr(Expr* expr);

			//inherit from UnaryExpr is enough
			//void Dump(std::ostream&) const;

			// Always int
			virtual QualType ReturnType() const;

			// Always RValue
			virtual ExprValueType ValueType() const;

			virtual bool Evaluatable() const;

			virtual ComposedValue Evaluate() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class SizeofTypeExpr : public Expr
		{
		public:
			SizeofTypeExpr(TypeExpr* type_expr);
			void Dump(std::ostream&) const;
			const TypeExpr* SubExpr() const { return m_TypeExpr.get(); }

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<TypeExpr> m_TypeExpr;
		};

		class MemberExpr : public Expr
		{
		public:
			static MemberExpr* MakeMemberExpr(Expr* host, const std::string &member_name, OperatorsEnum op);
			const Expr* Host() const
			{
				return m_HostExpr.get();
			}
			const FieldDeclaration* Member() const
			{
				return m_MemberDeclaration;
			}
			// "." is direct member operator
			// "->" is indirect member operator
			bool IsDirect() const;
			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual bool Evaluatable() const;

		protected:
			MemberExpr(Expr* host, const std::string &member_name, OperatorsEnum op);

			virtual void GenerateLValue(C1::PCode::CodeDome& dome);

			virtual void Generate(C1::PCode::CodeDome& dome);

			OperatorsEnum			m_Operator;
			std::unique_ptr<Expr>	m_HostExpr;
			std::string				m_MemberName;
			FieldDeclaration*		m_MemberDeclaration;
		};

		class CastExpr : public Expr
		{
		public:
			const Expr* SourceExpr() const { return m_SourceExpr.get(); }
			const QualType TargetType() const { return m_TargetType; }
			const FunctionDeclaration* ConversionFunction() const { return m_ConversionFunction; }
			void SetConversionFunction(FunctionDeclaration* val) { m_ConversionFunction = val; }

			static Expr* MakeImplictitCastExpr(QualType target_type, Expr* source_expr);
		protected:
			CastExpr(Expr* source_expr, QualType target_type, FunctionDeclaration* conversion_func = nullptr);

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

			virtual void Dump(std::ostream& ostr) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

			FunctionDeclaration* m_ConversionFunction;
			QualType m_TargetType;
			std::unique_ptr<Expr> m_SourceExpr;
		};
		class ImplicitCastExpr : public CastExpr
		{
		public:
			ImplicitCastExpr(QualType target_type, Expr* source_expr);
			void Dump(std::ostream&) const;
		};

		class ExplicitCastExpr : public CastExpr
		{
		public:
			ExplicitCastExpr(TypeExpr* target_type_expr, Expr* source_expr);
			const TypeExpr* TargetCastTypeExpr() const { return m_TargetTypeExpr.get(); }
			void Dump(std::ostream&) const;
		protected:
			std::unique_ptr<TypeExpr> m_TargetTypeExpr;
		};

		class ConstantLiteral : public Expr
		{
		public:
			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

			virtual void Dump(std::ostream& ostr) const;

			const std::string& RawLiteral() const { return m_RawLiteral; }

			void SetValue(ComposedValue val) { m_Value = val; }
		protected:
			ConstantLiteral(TypeContext* type_context, const std::string& raw_str);

			ComposedValue		m_Value;
			TypeContext*	m_TypeContext;
			std::string		m_RawLiteral;


		};

		class StringLiteral : public ConstantLiteral
		{
		public:
			explicit StringLiteral(TypeContext* type_context, const char* raw_str);

			virtual QualType ReturnType() const;

			// Not implemented yet
			static std::string DecodeString(const std::string & encoded_str);
			static std::string EncodeString(const std::string & raw_str);

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::string m_DecodedString; // The true value of this string literal
		};

		class IntegerLiteral : public ConstantLiteral
		{
		public:
			//explicit IntegerLiteral(int value, );
			IntegerLiteral(TypeContext* type_context, const char* raw_str, int value, int dec);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			int m_decimal;
		};

		class FloatLiteral : public ConstantLiteral
		{
		public:
			//explicit FloatLiteral(float value, bool is_double = false);
			FloatLiteral(TypeContext* type_context, const char* raw_str, float value);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class CharacterLiteral : public ConstantLiteral
		{
		public:
			CharacterLiteral(TypeContext* type_context, const char* raw_str, char value);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class BooleanLiteral : public ConstantLiteral
		{
		public:
			BooleanLiteral(TypeContext* type_context, const char* raw_str, bool value);

			virtual QualType ReturnType() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class Initializer : public Expr
		{
		public:
			virtual bool IsList() const = 0;
			//virtual void Generate(C1::PCode::CodeDome& dome) = delete;
		protected:
			Initializer();
		};

		class AtomInitializer : public Initializer
		{
		public:
			AtomInitializer(Expr* value_expr);
			bool IsList() const { return false; }

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

			virtual void Dump(std::ostream& ostr) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Expr> m_ValueExpr;
		};

		class InitializerList : public Initializer, public std::list<Initializer*>
		{
		public:
			//explicit InitializerList(std::list<Initializer*>& list);
			//explicit InitializerList(const std::list<Initializer*>& list);
			InitializerList(TypeContext *context);
			bool IsList() const { return true; }
			void Dump(std::ostream&) const;

			virtual QualType ReturnType() const;

			virtual ExprValueType ValueType() const;

			virtual bool HasSideEffect() const;

			virtual Expr::ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			TypeContext *m_TypeContext;
			QualType m_ReturnType;
		};

		class CommaExpr : public BinaryExpr
		{
		public:
			CommaExpr(Expr* lhs, Expr* rhs);

			virtual void Generate(C1::PCode::CodeDome& dome);

			virtual QualType ReturnType() const;

			virtual ComposedValue Evaluate() const;

			virtual bool Evaluatable() const;

		};

	}
}