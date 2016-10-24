#pragma once
#include "ast_node.h"
#include "declaration.h"
#include "stmt.h"

namespace C1
{
	namespace AST
	{

		class TypeSpecifier : public Node
		{
		public:
			Type* RepresentType() { return m_RepresentType; }
			const Type* RepresentType() const { return m_RepresentType; }
			void SetRepresentType(Type* type) { m_RepresentType = type; }
		protected:
			TypeSpecifier(Type* represent_type)
				: m_RepresentType(represent_type)
			{}
			TypeSpecifier()
				: m_RepresentType(nullptr)
			{}

			virtual void Dump(std::ostream& ostr) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

			Type* m_RepresentType;
		};

		class PrimaryTypeSpecifier : public TypeSpecifier
		{
		public:
			PrimaryTypeSpecifier(Type* type)
				: TypeSpecifier(type)
			{}
			void Dump(std::ostream&) const;
		};

		class AccessDeclaration : public Node
		{
			AccessibilityEnum Accessibility;
		};

		class BaseClassSpecifier : public Node
		{
			AccessibilityEnum Accessbility() const;
			bool IsVirtualBase() const;
			TypeSpecifier* BaseClass() const;
		};


		class StructBody :public Node, public ScopeNode, public DeclContext
		{
		public:
			StructBody();

			virtual void Dump(std::ostream& ostr) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

			// Not considering manual offset assign yet
			void GenerateFieldsLayout();
		};

		class ClassBody : public StructBody
		{
			std::list<BaseClassSpecifier*>& BaseClasses();
			std::list<ClassMethodDeclaration*> Constructors();
		};

		// represent a struct specifier
		// anything like "struct foo" or "struct foo {..}" or "struct {...}"
		class StructDeclaration : public TypeSpecifier, public TypeDeclaration, public Redeclarable<StructDeclaration>
		{
		public:
			StructDeclaration(const std::string& name, StructBody* definition);
			StructDeclaration(const std::string& name);
			StructDeclaration(StructBody* definition);
			StructDeclaration();

			virtual bool IsLiteralNode() const { return true; }

			const std::string & Name() const { return m_Name; }
			StructBody* Definition() { return m_Definition.get(); }
			const StructBody* Definition() const { return m_Definition.get(); }
			void SetDefinition(StructBody* val);
			bool IsAnonymous() const { return m_Name == "%anonymous"; }
			// Retrieve the latest previous definition before this declaration
			StructBody* LatestDefinition();
			const StructBody* LatestDefinition() const;
			void Dump(std::ostream&) const;

			virtual DeclContext::InsertionResult AddToContext(DeclContext& context);

		protected:
			std::string m_Name;
			std::unique_ptr<StructBody> m_Definition;
		};


		class EnumSpecifier : public TypeSpecifier
		{
		public:
			EnumSpecifier(const std::string& name, std::list<Enumerator*>* definition);
			EnumSpecifier(std::list<Enumerator*>* definition);
			EnumSpecifier(const std::string& name);
		};

		class TypedefNameSpecifier : public TypeSpecifier
		{
		public:
			TypedefNameSpecifier(DeclContext* pContext, const std::string& name);
			void Dump(std::ostream&) const;

			const std::string& Name() const { return m_Name; }
			void SetName(const std::string& name) { m_Name = name; }
			bool Rename(const std::string& newName);
		protected:
			std::string m_Name;
			TypeDeclaration* m_RefDecl;
		};

		class QualifiedTypeSpecifier : public Node
		{
		protected:
			int m_Qualifiers;
			std::unique_ptr<TypeSpecifier> m_TypeSpecifier;
		public:
			QualifiedTypeSpecifier(int, TypeSpecifier*);
			int Qualifiers() const { return m_Qualifiers; }
			TypeSpecifier* DeclTypeSpecifier() const { return m_TypeSpecifier.get(); }
			QualType RepresentType() const { return QualType(m_TypeSpecifier->RepresentType(), m_Qualifiers); }
			void Dump(std::ostream&) const;
		};

		template <typename T>
		class CompoundDeclaration : public DeclStmt
		{
		public:
			const QualifiedTypeSpecifier* DeclSpecifier() const { return m_QTSpecifier.get(); }
			const std::list<Declarator*>& DeclaratorList() const { return m_DeclaratorList; }
			const std::list<T*>& Declarations() const { return m_DeclarationList; }
			std::list<T*>& Declarations() { return m_DeclarationList; }
		protected:
			CompoundDeclaration(QualifiedTypeSpecifier* QTSpecifier, std::list<Declarator*> &&declarator_list);
			~CompoundDeclaration()
			{
				for (auto& declarator : m_DeclaratorList)
				{
					if (declarator)
					{
						delete declarator;
						declarator = 0;
					}
				}
			}
			void Dump(std::ostream& os) const;

			std::unique_ptr<QualifiedTypeSpecifier> m_QTSpecifier;
			// Compound Declaration don't owns any declaration , this is a lazy access list.
			std::list<T*> m_DeclarationList;
			// Compound Declaration owns all the declarators inside it.
			std::list<Declarator*>	m_DeclaratorList;
		};

		class VarDeclStmt : public CompoundDeclaration<VariableDeclaration>
		{
		public:
			VarDeclStmt(StorageClassSpecifierEnum, QualifiedTypeSpecifier*, std::list<Declarator*>*);
			StorageClassSpecifierEnum StorageClassSpecifier() const { return m_StorageSpecifier; }
			void SetStorageClassSpecifier(StorageClassSpecifierEnum val) { m_StorageSpecifier = val; }
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			StorageClassSpecifierEnum m_StorageSpecifier;
		};

		class TypedefStmt : public CompoundDeclaration<TypeDeclaration>
		{
		public:
			TypedefStmt(QualifiedTypeSpecifier*, std::list<Declarator*>*);
			void Dump(std::ostream&) const;
		};

		class FieldDeclStmt : public CompoundDeclaration<FieldDeclaration>
		{
		public:
			FieldDeclStmt(QualifiedTypeSpecifier*, std::list<Declarator*>*);

			virtual void Dump(std::ostream& ostr) const;

		};

	}
}

template <typename T>
void C1::AST::CompoundDeclaration<T>::Dump(std::ostream& os) const
{
	os << *m_QTSpecifier;
	for (auto declarator : DeclaratorList())
	{
		os << *declarator << ", ";
	}
	if (!DeclaratorList().empty())
		os << "\b\b;" << std::endl;
	else
		os << ";" << std::endl;
}

template <typename T>
C1::AST::CompoundDeclaration<T>::CompoundDeclaration(QualifiedTypeSpecifier* QTSpecifier, std::list<Declarator*> &&declarator_list) : m_DeclaratorList(std::move(declarator_list)), m_QTSpecifier(QTSpecifier)
{
	m_QTSpecifier->SetParent(this);
	for (auto dr : m_DeclaratorList)
	{
		dr->SetParent(this);
	}
}

