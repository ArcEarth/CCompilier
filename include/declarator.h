#pragma once

#include "ast_node.h"
#include "type.h"

namespace C1
{
	namespace AST
	{
		class Declarator : public Node
		{
		public:
			// The most "basic" declarator of this declarator
			Declarator* Atom();
			const Declarator* Atom() const { return m_Base.get(); }
			// The base declarator of this declarator
			// return nullptr if it's already the deepest layer
			Declarator* Base() { return m_Base.get(); }
			const Declarator* Base() const { return m_Base.get(); }

			virtual QualType DecorateType(QualType base_type) = 0;
		protected:
			Declarator(Declarator* base = nullptr);

			virtual void Generate(C1::PCode::CodeDome& dome);

			std::unique_ptr<Declarator> m_Base;
		};

		class ParenDeclarator : public Declarator
		{
		public:
			ParenDeclarator(Declarator* base);
			QualType DecorateType(QualType base_type)
			{
				return Base()->DecorateType(base_type);
			}
			void Dump(std::ostream&) const;
		};

		class EmptyDeclarator : public Declarator
		{
		public:
			EmptyDeclarator();
			virtual QualType DecorateType(QualType base_type);

			virtual void Dump(std::ostream& ostr) const;

		};

		class IdentifierDeclarator : public Declarator
		{
		public:
			IdentifierDeclarator(const std::string& name);
			const std::string& Identifier() const { return m_Name; }
			void SetIdentifier(const std::string& val);
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type);

		protected:
			std::string m_Name;
		};

		class PointerDeclarator : public Declarator
		{
		public:
			PointerDeclarator(int qualfier_mask, Declarator* base);
			int QualifierMask() const { return m_Qualifiers; }
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type);

		protected:
			int m_Qualifiers;
		};

		// I hate "int array[] = {a,b};"
		class ArrayDeclarator : public Declarator
		{
		public:
			ArrayDeclarator(Declarator* base, Expr* size);
			const Expr* SizeExpr() const { return m_SizeExpr.get(); }
			bool IsCompelete() const { return m_SizeExpr != nullptr; }
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type);

		protected:
			std::unique_ptr<Expr> m_SizeExpr;
		};

		class ParameterDeclaration;

		// A simple declaration container for parameters
		class ParameterList : public Node, public DeclContext
		{
		public:
			ParameterList();
			//override the original add 
			//InsertionResult add(ParameterDeclaration* param);
			void GenerateParameterLayout();
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class FunctionalDeclarator : public Declarator
		{
		public:
			FunctionalDeclarator(Declarator* base, ParameterList* param_list);
			const ParameterList& Parameters() const { return *m_Parameters; }
			ParameterList& Parameters() { return *m_Parameters; }
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type);

		protected:
			std::unique_ptr<ParameterList> m_Parameters;
		};

		// It's not really so good to recognize InitDeclarator as a "Declarator"
		class InitDeclarator : public Declarator
		{
		public:
			InitDeclarator(Declarator* declarator, Initializer* initializer);
			const Initializer* InitializeExpr() const { return m_Initializer.get(); }
			Initializer* InitializeExpr() { return m_Initializer.get(); }
			void SetInitializeExpr(Initializer* val) { m_Initializer.reset(val); }
			void Dump(std::ostream&) const;

			// Also handle the case "int array[] = {a,b}"
			QualType DecorateType(QualType base_type);
		protected:
			std::unique_ptr<Initializer> m_Initializer;
		};

		class FieldDeclarator : public Declarator
		{
		public:
			FieldDeclarator(Declarator* base, Expr* offset = nullptr);
			FieldDeclarator(Expr* offset);
			Expr* OffsetExpr() { return m_OffsetExpr.get(); }
			const Expr* OffsetExpr() const { return m_OffsetExpr.get(); }
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type);
		protected:
			std::unique_ptr<Expr> m_OffsetExpr;
		};

		// Well , enumerator is not some kind of declarator...
		class Enumerator : public Node
		{
		public:
			explicit Enumerator(const std::string &name, Expr* value_expr = nullptr);
			const std::string& Name() const { return m_Name; }
			const Expr* Value() const { return m_ValueExpr.get(); }
			void Dump(std::ostream&) const;

			virtual QualType DecorateType(QualType base_type)
			{
				throw std::logic_error("The method or operation is not implemented.");
			}

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::string m_Name;
			std::unique_ptr<Expr> m_ValueExpr;
		};

	}
}