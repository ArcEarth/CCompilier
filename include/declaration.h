#pragma once

#ifndef _DECLARATION_CONTEXT_
#define _DECLARATION_H_

#include <string>
#include <list>
#include <map>
#include <memory>
#include <iterator>
#include "ast_node.h"
#include "decl_context.h"
#include "type.h"

namespace C1
{
	namespace AST
	{

		template <typename DeclarationType>
		class Redeclarable
		{
		public:
			static const bool redeclarable = true;

			const DeclarationType* prev() const { return m_pPrev; }
			DeclarationType* prev() { return m_pPrev; }
			const DeclarationType* next() const { return m_pNext; }
			DeclarationType* next() { return m_pNext; }

			const DeclarationType* first() const
			{
				auto decl = this;
				while (decl->prev()) decl = decl->prev();
				return static_cast<const DeclarationType*>(decl);
			}
			DeclarationType* first()
			{
				auto decl = this;
				while (decl->prev()) decl = decl->prev();
				return static_cast<DeclarationType*>(decl);
			}
			const DeclarationType* last() const
			{
				auto decl = this;
				while (decl->next()) decl = decl->next();
				return static_cast<const DeclarationType*>(decl);
			}
			DeclarationType* last()
			{
				auto decl = this;
				while (decl->next()) decl = decl->next();
				return static_cast<DeclarationType*>(decl);
			}
			void add_last(DeclarationType* new_decl)
			{
				auto decl = last();
				decl->m_pNext = new_decl;
				new_decl->m_pPrev = decl;
			}
			void add_first(DeclarationType* new_decl)
			{
				auto decl = first();
				decl->m_pPrev = new_decl;
				new_decl->m_pNext = decl;
			}

			size_t redefination_count() const
			{
				size_t count = 1;
				DeclarationType* decl = this;
				while (decl->next()) {
					decl = decl->next();
					count++;
				}
				decl = this;
				while (decl->prev()) {
					decl = decl->prev();
					count++;
				}
				return count;
			}
		protected:
			DeclarationType *m_pPrev, *m_pNext;
		};

		// Represent an declaration in Semantic , the unit for lookup
		class Declaration
		{
		public:
			enum KindEnum{
				DECL_UNKNOWN,
				DECL_VALUE,
				DECL_VARIABLE,
				DECL_FIELD,
				DECL_PARAMETER,
				DECL_FUNCTION,
				DECL_TYPE,
				DECL_TYPEDEF,
				DECL_RECORD,
				DECL_STRUCT,
			};

			const KindEnum Kind() const
			{ return m_Kind; }

			// get the semantic affiliation of this declaration
			const DeclContext* Affiliation() const { return m_Affiliation; }
			DeclContext* Affiliation() { return m_Affiliation; }
			void SetAffiliation(DeclContext* affiliation) { m_Affiliation = affiliation; }

			// get the node which contains this declaration literally
			// return self for non-compound-declarations like ParameterDeclaration, FunctionDeclaration, StructDeclaration
			const Node* SourceNode() const { return m_Source; }
			Node* SourceNode() { return m_Source; }
			void SetSourceNode(Node* source) { m_Source = source; }

			// Invoked by DeclContext::add(Declaration*)
			virtual DeclContext::InsertionResult AddToContext(DeclContext& context);
			virtual ~Declaration();
			virtual bool IsLiteralNode() const { return false; }

			// Well , this is incorrect!!!
			static bool CheckCompatible(const Declaration* lsh,const Declaration* rhs);
		protected:
			Declaration(KindEnum kind = DECL_UNKNOWN, DeclContext* affiliation = nullptr)
				: m_Affiliation(nullptr), m_Kind(kind)
			{}

			void SetKind(KindEnum kind)
			{
				m_Kind = kind;
			}

			Node*	m_Source;
			KindEnum m_Kind;
			DeclContext* m_Affiliation;
		};

		class NamedDeclaration : public Declaration
		{
		public:
			const std::string& Name() const
			{
				return m_name;
			}

			void Rename(const std::string& name)
			{

			}

			void SetName(const std::string& name)
			{
				m_name = name;
			}
		protected:
			NamedDeclaration()
				: m_name("%anonymous")
			{}
			NamedDeclaration(const std::string& name)
				: m_name(name)
			{}

			virtual DeclContext::InsertionResult AddToContext(DeclContext& context);

			std::string m_name;
		};

		class ValueDeclaration : public NamedDeclaration
		{
		public:
			const QualType DeclType() const
				{ return m_decltype; }
			QualType DeclType()
				{ return m_decltype; }
			void SetDeclType(QualType qual_type)
				{ m_decltype = qual_type; }

			StorageClassSpecifierEnum StorageClassSpecifier() const 
				{ return m_storage_specifier; }

			const size_t Offset() const { return m_Offset; }
			void SetOffset(size_t offset) { m_Offset = offset; }

			// 			const Declarator* DeclDeclarator() const { return m_Declarator.get(); }
// 			Declarator* DeclDeclarator() { return m_Declarator.get(); }

		protected:
			ValueDeclaration();
			ValueDeclaration(StorageClassSpecifierEnum scs, QualType decl_type, const std::string& name);
			ValueDeclaration(StorageClassSpecifierEnum scs, QualType base_type, Declarator* declarator);
			~ValueDeclaration();

			//std::unique_ptr<Declarator>	m_Declarator;
			size_t m_Offset;
			StorageClassSpecifierEnum	m_storage_specifier;
			QualType					m_decltype;
		};

		// represent a local/global variable
		// A variable's declaration may not directly bind to an location since the exist of compound declaration
		class VariableDeclaration : public ValueDeclaration//, Redeclarable<VariableDeclaration>
		{
		public:
			VariableDeclaration(StorageClassSpecifierEnum storage_class_specifier, QualType qual_type, Declarator* p_declarator);
			// Use for declare a int declaration.
			VariableDeclaration(QualType decl_type, const std::string& name);
			const Initializer* InitializeExpr() const { return m_InitializerExpr; }
			Initializer* InitializeExpr() { return m_InitializerExpr; }
			bool ValidateInitialization();

			bool IsGlobal() const;

			//const size_t Address() const { return m_Address; }
			//void SetAddress(size_t val) { m_Address = val; }

		protected:
			VariableDeclaration();

			//size_t m_Address;
			Initializer*	m_InitializerExpr;
		};

		// Represent a struct field
		// A field's declaration may not directly bind to an location since the exist of compound declaration
		class FieldDeclaration : public ValueDeclaration//, Redeclarable<FieldDeclaration>
		{
		public:
			FieldDeclaration(QualType qual_type, Declarator* declarator);
			static const size_t	Offset_Auto = -1;

			const Type* ParentRecordType() const;
			const int AccessModifier() const;

			const Expr* OffsetExpr() const;
			
		protected:
			FieldDeclaration()
			{
				SetKind(DECL_FIELD);
			}

			Expr* m_OffsetExpr;
		};


		class QualifiedTypeSpecifier;
		class FunctionalDeclarator;
		class ParameterList;

		class FunctionDeclaration : public Node, public ValueDeclaration, public Redeclarable<FunctionDeclaration>
		{
		public:
			FunctionDeclaration(StorageClassSpecifierEnum scs, QualifiedTypeSpecifier *qualified_type_specifier, FunctionalDeclarator* declarator);
			~FunctionDeclaration();

			const QualifiedTypeSpecifier* DeclTypeSpecifier() const { return m_type_specifier.get(); }

			const FunctionalDeclarator* DeclDeclarator() const { return m_Declarator.get(); }
			FunctionalDeclarator* DeclDeclarator() { return m_Declarator.get(); }

			virtual bool IsLiteralNode() const { return true; }
			//const QualType FunctionType() const;
			//QualType FunctionType() ;

			Stmt* Definition()  { return m_Definition.get(); }
			const Stmt* Definition() const { return m_Definition.get(); }
			void SetDefinition(Stmt* def);
			Stmt* LatestDefinition();
			const Stmt* LatestDefinition() const;

			ParameterList& Parameters();
			QualType ReturnType();
			// It should always be negative
			int ReturnValueOffset();

			void Dump(std::ostream&) const;

			virtual DeclContext::InsertionResult AddToContext(DeclContext& context);

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			//Literal entities
			std::unique_ptr<QualifiedTypeSpecifier>	m_type_specifier;
			std::unique_ptr<FunctionalDeclarator>	m_Declarator;
			std::unique_ptr<Stmt>					m_Definition;
			//QualType								m_FunctionType;
		};

		// Represent an Parameter Declaration
		// Literal & Semantic Entity
		// Parameter's Declaration is not ambiguous , we can unify the literal and semantic entity
		class ParameterDeclaration : public Node, public ValueDeclaration
		{
		public:
			ParameterDeclaration(QualifiedTypeSpecifier*, Declarator*);
			~ParameterDeclaration();
			QualifiedTypeSpecifier* DeclSpecifier() { return m_QTSpecifier.get(); }
			bool IsNamed() const;
			void Dump(std::ostream&) const;

			const Declarator* DeclDeclarator() const { return m_Declarator.get(); }
			Declarator* DeclDeclarator() { return m_Declarator.get(); }
			void SetDeclarator(Declarator* val) { m_Declarator.reset(val); }

			virtual DeclContext::InsertionResult AddToContext(DeclContext& context);

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Declarator>				m_Declarator;
			std::unique_ptr<QualifiedTypeSpecifier> m_QTSpecifier;
		};

		class TypeDeclaration : public NamedDeclaration
		{
		public:
			const Type* DeclType() const { return m_type; }
			Type* DeclType() { return m_type; }
			void SetDeclType(Type* type)  { m_type = type; }
		protected:
			Type* m_type;
		};

		class TypedefDeclaration : public TypeDeclaration//, Redeclarable<TypedefDeclaration>
		{
		public:
			TypedefDeclaration(QualType qual_type, Declarator* p_declarator);
			//const Declarator* DeclDeclarator() const { return m_Declarator; }
			//Declarator* DeclDeclarator() { return m_Declarator; }

		protected:
			TypedefDeclaration()
			{
				SetKind(DECL_TYPEDEF);
			}
			//std::shared_ptr<QualifiedTypeSpecifier> m_QTSpecifier;
			//Declarator*	m_Declarator;
		};


		//class RecordDeclaration : public TypeDeclaration
		//{
		//public:
		//	RecordDeclaration(int record_keyword ,const std::string &name);
		//};
	}

}

#endif // !_DECLARATION_CONTEXT_

