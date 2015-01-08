#include "stdafx.h"

#include "decl_context.h"
#include "declaration.h"
#include "ast.h"

using namespace C1::AST;
using namespace std;

//DeclContext::InsertionResult DeclContext::add(Declaration* declaration)
//{
//	Indexize();
//
//	m_ListView.push_back(declaration);
//
//	declaration->SetAffiliation(this);
//	return Success;
//}


//C1::AST::DeclContext::InsertionResult C1::AST::DeclContext::add(ValueDeclaration* obj)
//{
//	add(obj,NameCollisonPolicy::AlwaysRedefinable);
//}

C1::AST::DeclContext::InsertionResult C1::AST::DeclContext::add(Declaration* declaration)
{
	Indexize();
	return declaration->AddToContext(*this);
}

void C1::AST::DeclContext::force_add(NamedDeclaration* decl)
{
	m_ListView.push_back(decl);
	if (m_pTableView) {
		std::pair<std::reference_wrapper<const std::string>, NamedDeclaration*> pa(decl->Name(), decl);
		m_pTableView->insert(pa);
	}
	decl->SetAffiliation(this);
}

void C1::AST::DeclContext::force_add(Declaration* decl)
{
	m_ListView.push_back(decl);
	decl->SetAffiliation(this);
}

C1::AST::DeclContext::~DeclContext()
{
	for (auto& decl : m_ListView)
	{
		// if it's an literal node, than we don't destroy it here
		if (!decl->IsLiteralNode())
		{
			delete decl;
			decl = nullptr;
		}
	}
}

void DeclContext::Indexize()
{
	if (m_pTableView || m_ListView.size() < IndexizeThreshold) return;
	m_pTableView .reset(new std::multimap<std::reference_wrapper<const std::string>, NamedDeclaration*>());
	for (auto itr = m_ListView.rbegin(); itr != m_ListView.rend(); ++itr)
	{
		auto pNameDecl = dynamic_cast<NamedDeclaration*>(*itr);
		std::pair<std::reference_wrapper<const std::string>, NamedDeclaration*> pa(pNameDecl->Name(), pNameDecl);
		m_pTableView->insert(pa);
	}
}

const NamedDeclaration* DeclContext::lookup_local(const std::string& name) const
{
	if (m_pTableView)
	{
		auto itr = m_pTableView->find(name);
		if (itr != m_pTableView->end())
			return itr->second;
		return nullptr;
	}
	else
	{
		auto itr = std::find_if(m_ListView.rbegin(), m_ListView.rend(),
			[&name](const Declaration* decl)
		{
			auto name_decl = dynamic_cast<const NamedDeclaration*>(decl);
			return name_decl && name_decl->Name() == name;
		});
		if (itr == m_ListView.rend())
			return nullptr;
		return static_cast<NamedDeclaration*>(*itr);
	}
}

const NamedDeclaration* DeclContext::lookup(const std::string &name) const
{
	const DeclContext* pContext = this;
	while (pContext)
	{
		auto pDecl = pContext->lookup_local(name);
		if (pDecl)
			return pDecl;
		else
			pContext = pContext->parent();
	}
	return nullptr;
}

size_t C1::AST::total_value_size(const DeclContext& context)
{
	size_t total_size = 0;
	for (auto decl : context)
	{
		auto var = dynamic_cast<const VariableDeclaration*>(decl);
		if (var)
		{
			total_size += var->DeclType()->Size();
		}
	}
	return total_size;
}
