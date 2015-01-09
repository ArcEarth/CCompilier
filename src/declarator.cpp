#include "stdafx.h"
#include "ast.h"
using namespace C1;
using namespace C1::AST;
using namespace std;

Declarator* C1::AST::Declarator::Atom()
{
	auto declarator = this;
	while (declarator->Base())
	{
		declarator = declarator->Base();
	}
	return declarator;
}

C1::AST::Declarator::Declarator(Declarator* base)
: m_Base(base)
{
	if (base) {
		base->SetParent(this);
		SetLocation(base->Location());
	}
}

void C1::AST::Declarator::Generate(C1::PCode::CodeDome& dome)
{
}

C1::AST::InitDeclarator::InitDeclarator(Declarator* declarator, Initializer* initializer)
: Declarator(declarator), m_Initializer(initializer)
{
}

void C1::AST::InitDeclarator::Dump(std::ostream& os) const
{
	os << *m_Base;
	if (InitializeExpr())
	{
		os << " = " << *m_Initializer;
	}
}

C1::AST::QualType C1::AST::InitDeclarator::DecorateType(QualType base_type)
{
	if (m_Initializer->IsList())
	{
		auto list = dynamic_cast<InitializerList*>(m_Initializer.get());
		//auto arr_base = dynamic_cast<ArrayDeclarator*>(Base());

		auto res_type = Base()->DecorateType(base_type);
		auto arr_type = res_type.As<ArrayType>();
		if (arr_type) {
			if (arr_type->Length() == ArrayType::ArrayLengthAuto)
				arr_type->SetLength(list->size());
			if (arr_type->Length() < static_cast<int>(list->size()))
				error(this, "Initializer-list size don't match the object-array size");
		}
		else if (res_type->IsStructType())
		{
			auto struct_type = res_type.As<StructType>();
			if (!struct_type->Definition())
			{
				error(this, "Can not instantiate a incomplete type");
			}
			else
			{
				if (!(struct_type <= list->ReturnType()))
				{
					error(this, "Initializer List type don't match struct type.");
				}
			}
		}
		else
		{
			error(this, "Initializer-list can only initialize array or struct type object");
		}
		return res_type;
	}
	return Base()->DecorateType(base_type);
}

C1::AST::ParenDeclarator::ParenDeclarator(Declarator* base)
: Declarator(base)
{
}

void C1::AST::ParenDeclarator::Dump(std::ostream& os) const
{
	os << "(" << *m_Base << ")";
}

C1::AST::IdentifierDeclarator::IdentifierDeclarator(const std::string& name)
: Declarator(nullptr), m_Name(name)
{

}

void C1::AST::IdentifierDeclarator::Dump(std::ostream& os) const
{
	os << m_Name;
}

void C1::AST::IdentifierDeclarator::SetIdentifier(const std::string& val)
{
	m_Name = val;
}

C1::AST::QualType C1::AST::IdentifierDeclarator::DecorateType(QualType base_type)
{
	return base_type;
}

C1::AST::PrefixDeclarator::PrefixDeclarator(OperatorsEnum prefix, int qualfier_mask, Declarator* base)
: Declarator(base), m_Operator(prefix), m_Qualifiers(qualfier_mask)
{
}

void C1::AST::PrefixDeclarator::Dump(std::ostream& os) const
{
	os << m_Operator << QualType::QulifierMaskToString(m_Qualifiers) << *m_Base;
}

C1::AST::QualType C1::AST::PrefixDeclarator::DecorateType(QualType base_type)
{
	auto decoratedType = QualType(MakePointer(base_type), QualifierMask());
	if (m_Base)
		decoratedType = m_Base->DecorateType(decoratedType);
	return decoratedType;
}

C1::AST::ArrayDeclarator::ArrayDeclarator(Declarator* base, Expr* size)
: Declarator(base), m_SizeExpr(size)
{
	//assert(size->Evaluatable());
}

void C1::AST::ArrayDeclarator::Dump(std::ostream& os) const
{
	os << *m_Base;
	if (m_SizeExpr)
		os << "[" << *m_SizeExpr << "]";
	else
		os << "[]";
}

C1::AST::QualType C1::AST::ArrayDeclarator::DecorateType(QualType base_type)
{
	size_t length = ArrayType::ArrayLengthAuto;
	if (SizeExpr() && SizeExpr()->Evaluatable()) {
		length = SizeExpr()->Evaluate().Int;
	}
	auto decoratedType = QualType(MakeArray(base_type, length));
	if (m_Base)
		decoratedType = m_Base->DecorateType(decoratedType);
	return decoratedType;
}

C1::AST::FunctionalDeclarator::FunctionalDeclarator(Declarator* base, ParameterList* param_list, int qualfier_mask)
: Declarator(base), m_Parameters(param_list)
{

}

void C1::AST::FunctionalDeclarator::Dump(std::ostream& os) const
{
	os << *Base() << *m_Parameters << QualType::QulifierMaskToString(m_Qualifiers);
}

C1::AST::QualType C1::AST::FunctionalDeclarator::DecorateType(QualType base_type)
{
	auto Context = base_type->AffiliatedContext();
	std::list<QualType> parameterTypes;
	for (auto decl : Parameters())
	{
		auto param = dynamic_cast<ParameterDeclaration*>(decl);
		parameterTypes.push_back(param->DeclType());
	}
	auto decoratedType = QualType(Context->NewFunctionType(base_type, std::move(parameterTypes)));
	if (m_Base)
		decoratedType = m_Base->DecorateType(decoratedType);
	return decoratedType;
}

C1::AST::FieldDeclarator::FieldDeclarator(Declarator* base, Expr* offset /*= nullptr*/)
: Declarator(base), m_OffsetExpr(offset)
{
	if (m_OffsetExpr)
		assert(m_OffsetExpr->Evaluatable() && m_OffsetExpr->ReturnType()->IsIntegerType());
}

C1::AST::FieldDeclarator::FieldDeclarator(Expr* offset)
: Declarator(nullptr), m_OffsetExpr(offset)
{
	assert(m_OffsetExpr);
	assert(m_OffsetExpr->Evaluatable() && m_OffsetExpr->ReturnType()->IsIntegerType());
}

void C1::AST::FieldDeclarator::Dump(std::ostream& os) const
{
	os << *Base();
	if (m_OffsetExpr)
		os << " : " << *m_OffsetExpr;
}

C1::AST::QualType C1::AST::FieldDeclarator::DecorateType(QualType base_type)
{
	return Base()->DecorateType(base_type);
}

C1::AST::Enumerator::Enumerator(const std::string &name, Expr* value_expr /*= nullptr*/)
: m_Name(name), m_ValueExpr(value_expr)
{
}

void C1::AST::Enumerator::Dump(std::ostream& os) const
{
	os << m_Name;
	if (m_ValueExpr)
		os << " = " << *m_ValueExpr;
}

void C1::AST::Enumerator::Generate(C1::PCode::CodeDome& dome)
{
}

C1::AST::QualType C1::AST::EmptyDeclarator::DecorateType(QualType base_type)
{
	return base_type;
}

void C1::AST::EmptyDeclarator::Dump(std::ostream& ostr) const
{
}

C1::AST::EmptyDeclarator::EmptyDeclarator()
{

}
