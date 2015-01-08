#include "stdafx.h"
#include "ast.h"
using namespace C1;
using namespace C1::AST;
using namespace std;
using namespace C1::PCode;

void C1::AST::PrimaryTypeSpecifier::Dump(std::ostream& os) const
{
	os << RepresentType()->ToString() << " ";
}

void C1::AST::StructBody::Dump(std::ostream& os) const
{
	os << "{" << endl;
	for (auto child : Children())
	{
		os << *child;
	}
	os << "}";
}

C1::AST::StructBody::StructBody()
{

}

void C1::AST::StructBody::Generate(C1::PCode::CodeDome& dome)
{
	GenerateFieldsLayout();
}

void C1::AST::StructBody::GenerateFieldsLayout()
{
	size_t local_var_size = 0;
	for (auto decl : *this)
	{
		auto field = dynamic_cast<FieldDeclaration*>(decl);
		if (field)
		{
			field->SetOffset(local_var_size);
			local_var_size += field->DeclType()->Size();
		}
	}
}

C1::AST::StructDeclaration::StructDeclaration(const std::string& name, StructBody* definition)
: m_Name(name), m_Definition(definition)
{
	SetKind(DECL_STRUCT);
	definition->SetParent(this);
}

C1::AST::StructDeclaration::StructDeclaration(const std::string& name)
: m_Name(name), m_Definition(nullptr)
{
	SetKind(DECL_STRUCT);
}

C1::AST::StructDeclaration::StructDeclaration(StructBody* definition)
: m_Name("%anonymous"), m_Definition(definition)
{
	SetKind(DECL_STRUCT);
	definition->SetParent(this);
}

C1::AST::StructDeclaration::StructDeclaration()
{
	SetKind(DECL_STRUCT);
}

StructBody* C1::AST::StructDeclaration::LatestDefinition()
{
	auto decl = this;
	while (decl && decl->Definition() == nullptr)
	{
		decl = decl->prev();
	}
	if (decl)
		return decl->Definition();
	else
		return nullptr;
}

const StructBody* C1::AST::StructDeclaration::LatestDefinition() const
{
	auto decl = this;
	while (decl && decl->Definition() == nullptr)
	{
		decl = decl->prev();
	}
	if (decl)
		return decl->Definition();
	else
		return nullptr;
}

void C1::AST::StructDeclaration::Dump(std::ostream& os) const
{
	os << "struct ";
	if (!IsAnonymous()) os << m_Name << " ";
	if (Definition()) os << *Definition() << " ";
}

DeclContext::InsertionResult C1::AST::StructDeclaration::AddToContext(DeclContext& context)
{
	using InsertionResult = DeclContext::InsertionResult;
	using NameCollisonPolicy = DeclContext::NameCollisonPolicy;
	const auto name_policy = NameCollisonPolicy::CompatibleRedefinable;

	InsertionResult result = InsertionResult::Success;
	auto decl = context.lookup<StructDeclaration>(this->Name());
	if (decl)
	{
		if (Declaration::CheckCompatible(decl, this))
		{
			result = InsertionResult::Success_CompatibleRedefinition;
			decl->add_last(this);
			this->SetDeclType(decl->DeclType());
			this->SetRepresentType(decl->DeclType());
		}
		else
		{
			result = InsertionResult::Success_IncompatibleRedefinition;
			auto prev_decl = dynamic_cast<StructDeclaration*>(decl);
			if (prev_decl) prev_decl->add_last(this);
		}
	}
	else
	{

	}

	context.force_add(this);
	return result;
}

void C1::AST::StructDeclaration::SetDefinition(StructBody* val)
{
	m_Definition.reset(val);
	val->SetParent(this);
	m_Definition->GenerateFieldsLayout();
	if (DeclType())
	{
		dynamic_cast<StructType*>(DeclType())->SetDefinition(val);
	}
}

void C1::AST::TypedefNameSpecifier::Dump(std::ostream& os) const
{
	auto decl_type = dynamic_cast<AliasType*>(m_RefDecl->DeclType());
	assert(decl_type);
	os << decl_type->Name() << " ";
	//os << m_Name << " ";
}

C1::AST::TypedefNameSpecifier::TypedefNameSpecifier(DeclContext* pContext, const std::string& name) : m_Name(name)
{
	TypeDeclaration* decl = dynamic_cast<TypeDeclaration*>(pContext->lookup(name));
	m_RefDecl = decl;
	if (decl)
	{
		SetRepresentType(decl->DeclType());
	}
}

C1::AST::QualifiedTypeSpecifier::QualifiedTypeSpecifier(int qm, TypeSpecifier* ts)
: m_Qualifiers(qm), m_TypeSpecifier(ts)
{
	ts->SetParent(this);
}

void C1::AST::QualifiedTypeSpecifier::Dump(std::ostream& os) const
{
	os << QualType::QulifierMaskToString(m_Qualifiers) << *m_TypeSpecifier;
}

C1::AST::VarDeclStmt::VarDeclStmt(StorageClassSpecifierEnum scs, QualifiedTypeSpecifier* qts, std::list<Declarator*>* dlist)
: m_StorageSpecifier(scs), CompoundDeclaration(qts, std::move(*dlist))
{
}

void C1::AST::VarDeclStmt::Dump(std::ostream& os) const
{
	os << m_StorageSpecifier;
	CompoundDeclaration::Dump(os);
}

void C1::AST::VarDeclStmt::Generate(C1::PCode::CodeDome& dome)
{
	for (auto var : m_DeclarationList)
	{
		if (!var->InitializeExpr())
			dome << gen(isp, 0, var->DeclType()->Size());
		else
			dome << *var->InitializeExpr();
	}
}

C1::AST::TypedefStmt::TypedefStmt(QualifiedTypeSpecifier* qts, std::list<Declarator*>* dlist)
: CompoundDeclaration(qts, std::move(*dlist))
{
}

void C1::AST::TypedefStmt::Dump(std::ostream& os) const
{
	os << "typedef ";
	CompoundDeclaration::Dump(os);
}

void C1::AST::FieldDeclStmt::Dump(std::ostream& os) const
{
	CompoundDeclaration::Dump(os);
}

C1::AST::FieldDeclStmt::FieldDeclStmt(QualifiedTypeSpecifier* qts, std::list<Declarator*>* dlist)
: CompoundDeclaration(qts, std::move(*dlist))
{
}

void C1::AST::TypeSpecifier::Dump(std::ostream& ostr) const
{
	ostr << "type_specifier";
}

void C1::AST::TypeSpecifier::Generate(C1::PCode::CodeDome& dome)
{
}


void C1::AST::DeclStmt::Generate(C1::PCode::CodeDome& dome)
{
}
