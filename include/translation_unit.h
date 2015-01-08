#pragma once
#include "ast_node.h"
#include "decl_context.h"

namespace C1
{
	namespace AST
	{

		class TranslationUnit : public Node, public ScopeNode, public DeclContext
		{
		public:
			TranslationUnit(std::string filename);
			~TranslationUnit();

			const std::string & Name() const { return m_FileName; }
			virtual void Dump(std::ostream& ostr) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::string m_FileName;
		};

	}
}