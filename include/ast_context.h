#pragma once

#include "decl_context.h"
#include "type.h"
#include <stack>

namespace C1
{
	namespace AST
	{
		class Type;
		class DeclContext;
		class Enumerator;
		class QualifiedTypeSpecifier;
		class TranslationUnit;

		struct ASTContext
		{
		public:
			// The current translation unit
			TranslationUnit* CurrentTranslationUnit;
			// The name of the current working source file
			std::string		FileName;
			// The input stream for source file
			std::istream*	SourceFile;
			// Storage the qualified type for declarations.
			QualType		CurrentQualifiedType;

			// Storage the current working scope , the coming declaration is stored in it
			//DeclContext*	CurrentDeclContext();

			// The type context object for creating and retrieving predefined types
			TypeContext*	type_context;

			void push_context(DeclContext* decl_context)
			{
				if (!m_DeclStack.empty())
					decl_context->set_parent(m_DeclStack.top());
				else
					decl_context->set_parent(nullptr);
				m_DeclStack.push(decl_context);
			}

			DeclContext* pop_context()
			{
				auto decl_context = m_DeclStack.top();
				m_DeclStack.pop();
				return decl_context;
			}

			DeclContext* current_context()
			{
				return m_DeclStack.top();
			}

			const DeclContext* current_context() const
			{
				return m_DeclStack.top();
			}
		protected:
			std::stack<DeclContext*>	m_DeclStack;
		};
	}

}