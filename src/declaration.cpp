#include "stdafx.h"

#include "declaration.h"
#include "decl_context.h"
#include "ast.h"

using namespace C1;
using namespace C1::AST;
using namespace std;


ParameterList& C1::AST::FunctionDeclaration::Parameters()
{
	return m_Declarator->Parameters();
}

void C1::AST::ParameterList::Dump(std::ostream& os) const
{
	if (empty()){
		os << "()";
		return;
	}
	os << "(";
	for (auto param : *this)
	{
		os << *static_cast<ParameterDeclaration*>(param) << ", ";
	}
	os << "\b\b)";
}

C1::AST::ParameterList::ParameterList()
{
}

void C1::AST::ParameterList::GenerateParameterLayout()
{
	int base = 0;// -static_cast<int>(ReturnValueSize);
	for (auto decl : *this)
	{
		auto param = dynamic_cast<ParameterDeclaration*>(decl);
		if (param)
		{
			base -= param->DeclType()->Size();
			param->SetOffset(base);
		}
	}
}

void C1::AST::ParameterList::Generate(C1::PCode::CodeDome& dome)
{
	GenerateParameterLayout();
}


C1::AST::QualType C1::AST::FunctionDeclaration::ReturnType()
{
	return dynamic_cast<FunctionType*>(DeclType().get())->ReturnType();
}

void C1::AST::FunctionDeclaration::Dump(std::ostream& os) const
{
	os << m_storage_specifier << *m_type_specifier << *m_Declarator;
	if (m_Definition)
		os << endl << *m_Definition;
	else
		os << ";" << endl;
}

DeclContext::InsertionResult C1::AST::FunctionDeclaration::AddToContext(DeclContext& context)
{
	using InsertionResult = DeclContext::InsertionResult;
	using NameCollisonPolicy = DeclContext::NameCollisonPolicy;
	const auto name_policy = NameCollisonPolicy::CompatibleRedefinable;

	InsertionResult result = InsertionResult::Success;
	auto decl = context.lookup<FunctionDeclaration>(this->Name());
	if (decl)
	{
		if (Declaration::CheckCompatible(decl, this))
		{
			result = InsertionResult::Success_CompatibleRedefinition;
			decl->add_last(this);
		}
		else
		{
			result = InsertionResult::Success_IncompatibleRedefinition;
			auto prev_func = dynamic_cast<FunctionDeclaration*>(decl);
			if (prev_func) prev_func->add_last(this);
		}
	}

	context.force_add(this);
	return result;
}

Stmt* C1::AST::FunctionDeclaration::LatestDefinition()
{
	auto func = this;
	while (!func->Definition())
	{
		func = func->prev();
	}
	return func->Definition();
}

const Stmt* C1::AST::FunctionDeclaration::LatestDefinition() const
{
	auto func = this;
	while (func && !func->Definition())
	{
		func = func->prev();
	}
	if (!func) 
		return nullptr;
	return func->Definition();
}

C1::AST::FunctionDeclaration::~FunctionDeclaration()
{
	SetKind(DECL_FUNCTION);
}

C1::AST::FunctionDeclaration::FunctionDeclaration(StorageClassSpecifierEnum scs, QualifiedTypeSpecifier *qualified_type_specifier, FunctionalDeclarator* declarator)
: ValueDeclaration(scs, qualified_type_specifier->RepresentType(), declarator), m_type_specifier(qualified_type_specifier), m_Declarator(declarator)
{
	SetKind(DECL_FUNCTION);
	SetSourceNode(this);
	//m_decltype = m_Declarator->DecorateType(qualified_type_specifier->RepresentType());
}

void C1::AST::FunctionDeclaration::SetDefinition(Stmt* def)
{
	m_Definition.reset(def);
	def->SetParent(this);
}

bool C1::AST::FunctionDeclaration::Rename(const std::string& name) 
{
	if (Affiliation()->rename_decl(this, name))
	{
		m_name = name;
		return true;
	}
	return false;
}

void C1::AST::FunctionDeclaration::Generate(C1::PCode::CodeDome& dome)
{
	if (m_Definition)
	{
		Parameters().Generate(dome);
		SetOffset(dome.InstructionCount());
		dome << PCode::gen(PCode::isp, 0, PCode::CodeDome::CallStorageSize);
		int spx = dome.SP;
		dome.SP = PCode::CodeDome::CallStorageSize;
		dome << *m_Definition;
		dome.SP = spx;
		dome << PCode::gen(PCode::opr, 0, OP_RET);
	}
}

int C1::AST::FunctionDeclaration::ReturnValueOffset()
{
	auto param_size = 0;
	for (auto decl : Parameters())
	{
		auto param = dynamic_cast<ParameterDeclaration*>(decl);
		param_size += param->DeclType()->Size();
	}
	param_size += ReturnType()->Size();
	return -param_size;
}

C1::AST::ParameterDeclaration::ParameterDeclaration(QualifiedTypeSpecifier* qts, Declarator* dr)
: ValueDeclaration(SCS_NONE,qts->RepresentType(),dr), m_QTSpecifier(qts), m_Declarator(dr)
{
	SetKind(DECL_PARAMETER);
	SetSourceNode(this);
	//SetDeclType(dr->DecorateType(qts->RepresentType()));
}

void C1::AST::ParameterDeclaration::Dump(std::ostream& os) const
{
	os << *m_QTSpecifier << *m_Declarator;
}

bool C1::AST::ParameterDeclaration::IsNamed() const
{
	return m_Declarator && dynamic_cast<IdentifierDeclarator*>(m_Declarator->Atom());
}

DeclContext::InsertionResult C1::AST::ParameterDeclaration::AddToContext(DeclContext& context)
{
	if (IsNamed())
		return NamedDeclaration::AddToContext(context);
	else
		return Declaration::AddToContext(context);
}

C1::AST::ParameterDeclaration::~ParameterDeclaration()
{

}

void C1::AST::ParameterDeclaration::Generate(C1::PCode::CodeDome& dome)
{
}

DeclContext::InsertionResult C1::AST::NamedDeclaration::AddToContext(DeclContext& context)
{
	using InsertionResult = DeclContext::InsertionResult;
	const DeclContext::NameCollisonPolicy name_policy = DeclContext::NameCollisonPolicy::AlwaysRedefinable;
	InsertionResult result = InsertionResult::Success;
	auto decl = context.lookup_local(Name());
	if (decl)
	{
		if (Declaration::CheckCompatible(decl, this))
			result = InsertionResult::Success_CompatibleRedefinition;
		else
			result = InsertionResult::Success_IncompatibleRedefinition;
	}
	if (result > name_policy)
		return InsertionResult(result + 2);

	context.force_add(this);
	return result;
}

DeclContext::InsertionResult C1::AST::Declaration::AddToContext(DeclContext& context)
{
	context.force_add(this);
	return DeclContext::Success;
}

bool C1::AST::Declaration::CheckCompatible(const Declaration* lhs, const Declaration* rhs)
{
	if (lhs->Kind() != rhs->Kind())
		return false;

	// Only Struct and function declaration is redeclarable
	if (lhs->Kind() != DECL_FUNCTION && lhs->Kind() != DECL_STRUCT)
		return false;
	if (lhs->Kind() == DECL_STRUCT)
	{
		auto ldec = dynamic_cast<const StructDeclaration*>(lhs);
		auto rdec = dynamic_cast<const StructDeclaration*>(rhs);
		if (ldec->LatestDefinition() && rdec->Definition())
			return false;
		return true;
	}
	else
	{
		auto ldec = dynamic_cast<const FunctionDeclaration*>(lhs);
		auto rdec = dynamic_cast<const FunctionDeclaration*>(rhs);
		if (ldec->LatestDefinition() && rdec->Definition())
			return false;
		return true;
	}
}

C1::AST::Declaration::~Declaration()
{

}

C1::AST::TypedefDeclaration::TypedefDeclaration(QualType qual_type, Declarator* p_declarator)
{
	SetKind(DECL_TYPEDEF);
	SetSourceNode(p_declarator);
	SetName(dynamic_cast<IdentifierDeclarator*>(p_declarator->Atom())->Identifier());
	auto type = MakeAlias(p_declarator->DecorateType(qual_type), Name());
	SetDeclType(type);
}

C1::AST::ValueDeclaration::ValueDeclaration(StorageClassSpecifierEnum scs, QualType decl_type, const std::string& name)
: NamedDeclaration(name), m_storage_specifier(scs), m_decltype(decl_type)
{
	m_Offset = -1;
}

C1::AST::ValueDeclaration::ValueDeclaration(StorageClassSpecifierEnum scs, QualType base_type, Declarator* declarator)
: m_storage_specifier(scs)
{
	m_Offset = -1;
	SetKind(DECL_VALUE);
	SetSourceNode(declarator);
	SetDeclType(declarator->DecorateType(base_type));
	auto id = dynamic_cast<IdentifierDeclarator*>(declarator->Atom());
	if (id)
		SetName(id->Identifier());
}

C1::AST::ValueDeclaration::ValueDeclaration()
{
	m_Offset = -1;
	SetKind(DECL_VALUE);
}

C1::AST::ValueDeclaration::~ValueDeclaration()
{

}

C1::AST::FieldDeclaration::FieldDeclaration(QualType qual_type, Declarator* declarator)
: ValueDeclaration(SCS_NONE,qual_type,declarator)
{
	SetKind(Declaration::DECL_FIELD);
	m_OffsetExpr = nullptr;
	m_Offset = Offset_Auto;
	auto field = dynamic_cast<FieldDeclarator*>(declarator);
	if (field && field->OffsetExpr())
	{
		m_OffsetExpr = field->OffsetExpr();
		if (field->OffsetExpr()->Evaluatable())
			m_Offset = field->OffsetExpr()->Evaluate().Int;
	}
}

//const size_t C1::AST::FieldDeclaration::Offset() const
//{
//	return m_Offset;
//}

C1::AST::VariableDeclaration::VariableDeclaration(StorageClassSpecifierEnum storage_class_specifier, QualType qual_type, Declarator* p_declarator)
: ValueDeclaration(storage_class_specifier, qual_type, p_declarator)
{
	SetKind(DECL_VARIABLE);
	m_InitializerExpr = nullptr;
	auto init = dynamic_cast<InitDeclarator*>(p_declarator);
	if (init)
	{
			m_InitializerExpr = init->InitializeExpr();
	}
}

C1::AST::VariableDeclaration::VariableDeclaration()
{
	SetKind(DECL_VARIABLE);
}

C1::AST::VariableDeclaration::VariableDeclaration(QualType decl_type, const std::string& name)
: ValueDeclaration(SCS_NONE, decl_type, name)
{
	SetKind(DECL_VARIABLE);
}

bool C1::AST::VariableDeclaration::ValidateInitialization()
{
	if (!InitializeExpr())
		return true;
	auto varType = DeclType();
	auto initializerType = InitializeExpr()->ReturnType();
	varType.RemoveConst();
	if (!is_type_assignable(varType, initializerType))
	{
		error(SourceNode(), "Can not convert initializer type from <r-type> to <l-type>");
		return false;
	}
	else
		return true;

}

bool C1::AST::VariableDeclaration::IsGlobal() const
{
	return m_Affiliation->parent() == nullptr;
}
