#include "stdafx.h"
#include "ast.h"
#include "pcode.h"
using namespace C1;
using namespace C1::AST;
using namespace std;
using namespace C1::PCode;


C1::AST::Expr::ComposedValue C1::AST::Expr::Evaluate() const
{
	ComposedValue val;
	val.Int = 0;
	return val;
}

void C1::AST::Expr::GenerateLValue(C1::PCode::CodeDome& dome)
{
}

C1::AST::Initializer::Initializer()
{

}

void BinaryExpr::Dump(ostream& os) const
{
	os << *LeftSubExpr() << ' ' << Operator() << ' ' << *RightSubExpr();
}

C1::AST::BinaryExpr::BinaryExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
: m_Operator(op)
{
	lhs->SetParent(this);
	rhs->SetParent(this);
	SetLocation(lhs->Location() + rhs->Location());
	m_SubExprs[0].reset(lhs); m_SubExprs[1].reset(rhs);
}

C1::AST::QualType C1::AST::BinaryExpr::ReturnType() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

C1::AST::ExprValueType C1::AST::BinaryExpr::ValueType() const
{
	return XValue;
}

bool C1::AST::BinaryExpr::HasSideEffect() const
{
	return LeftSubExpr()->HasSideEffect() && RightSubExpr()->HasSideEffect();
}

Expr::ComposedValue C1::AST::BinaryExpr::Evaluate() const
{
	ComposedValue val;
	val.Int = 0;
	return val;
}

bool C1::AST::BinaryExpr::Evaluatable() const
{
	return LeftSubExpr()->Evaluatable() && RightSubExpr()->Evaluatable();
}

void C1::AST::PosfixExpr::Dump(std::ostream& os) const
{
	os << *SubExpr() << Operator();
}

C1::AST::PosfixExpr::PosfixExpr(OperatorsEnum op, Expr* Base)
: UnaryExpr(op, Base)
{
	assert(op == OP_POSFIX_ADDADD || op == OP_POSFIX_SUBSUB);
}

PosfixExpr* C1::AST::PosfixExpr::MakePosfixExpr(OperatorsEnum op, Expr* sub_expr)
{
	auto expr = new PosfixExpr(op, sub_expr);
	auto sub_type = sub_expr->ReturnType();
	if (!sub_type->IsIntegerType() && !sub_type->IsPointerType())
	{
		error(expr, "can not find overload of postfix operator (++/--) which takes arguments type (<sub-expr-type>)");
	}
	if (sub_type.IsConst())
	{
		error(expr, "operand of postfix operator (++/--) can not be an const type");
	}
	if (sub_expr->ValueType())
	{
		error(expr, "operand of postfix operator (++/--) must be a L-Value expression");
	}
	return expr;
}

C1::AST::ExprValueType C1::AST::PosfixExpr::ValueType() const
{
	return XValue;
}

void C1::AST::PosfixExpr::Generate(C1::PCode::CodeDome& dome)
{
	m_SubExpr->Generate(dome); // RValue
	m_SubExpr->GenerateLValue(dome);
	dome.EmplaceInstruction(opr, 0, m_Operator);

}

C1::AST::CallExpr::CallExpr(Expr* func, std::list<Expr*>* args)
: m_FuncExpr(func), m_Arguments(std::move(*args))
{

}

void C1::AST::CallExpr::Dump(std::ostream& os) const
{
	os << *m_FuncExpr;
	if (m_Arguments.empty()) {
		os << "()";
		return;
	}

	os << '(';
	for (auto expr : m_Arguments)
	{
		os << *expr << ", ";
	}
	os << "\b\b)";
}

C1::AST::CallExpr::~CallExpr()
{
	for (auto expr : m_Arguments)
	{
		if (expr) delete expr;
	}
}

QualType C1::AST::CallExpr::ReturnType() const
{
	return m_FuncExpr->ReturnType().As<FunctionType>()->ReturnType();
}

C1::AST::ExprValueType C1::AST::CallExpr::ValueType() const
{
	return XValue;
}

bool C1::AST::CallExpr::HasSideEffect() const
{
	return true;
}

bool C1::AST::CallExpr::Evaluatable() const
{
	return false;
}

CallExpr* C1::AST::CallExpr::MakeCallExpr(Expr* callee, std::list<Expr*>* args)
{
	auto expr = new CallExpr(callee, args);
	if (callee->ValueType() != LValue)
	{
		error(expr, "callable object must have a L-Value");
	}
	return expr;
}

void C1::AST::CallExpr::Generate(C1::PCode::CodeDome& dome)
{
	auto return_val_size = m_FuncExpr->ReturnType().As<FunctionType>()->ReturnType()->Size();
	dome.EmplaceInstruction(isp, 0, return_val_size);
	size_t args_size = 0;
	for (auto itr = m_Arguments.rbegin(); itr != m_Arguments.rend(); itr++)
	{
		dome << **itr;
		args_size += (*itr)->ReturnType()->Size();
	}
	m_FuncExpr->GenerateLValue(dome);
	// 1 means load stack top to the offset
	dome.EmplaceInstruction(cal, 1, 0);
	dome.EmplaceInstruction(isp, 0, -static_cast<int>(args_size));
}

C1::AST::DeclRefExpr::DeclRefExpr(DeclContext* lookup_context, TypeContext* type_context, const std::string &name)
: m_Name(name), m_RefContext(lookup_context), m_TypeContext(type_context)
{
	ResoloveReference();
}

void C1::AST::DeclRefExpr::Dump(std::ostream& os) const
{
	if (m_RefDecl)
		os << m_RefDecl->Name();
	else
		os << "<err-ref>(" << m_Name << ")";
}

ValueDeclaration* C1::AST::DeclRefExpr::ResoloveReference()
{
	auto decl = m_RefContext->lookup(m_Name);
	m_RefDecl = dynamic_cast<ValueDeclaration*>(decl);
	if (!m_RefDecl)
	{
		error(this, "Can not find the declaration of name ""<name>"", assuming int");
		m_RefDecl = new VariableDeclaration(QualType(m_TypeContext->Int()), m_Name);
		m_RefContext->add(m_RefDecl);
	}
	return m_RefDecl;
}

void C1::AST::DeclRefExpr::SetRefDeclContext(DeclContext* reference_context)
{
	m_RefContext = reference_context;
	ResoloveReference();
}

void C1::AST::DeclRefExpr::SetIdentifier(const std::string val)
{
	m_Name = val;
	ResoloveReference();
}

C1::AST::QualType C1::AST::DeclRefExpr::ReturnType() const
{
	return RefDecl()->DeclType();
}

C1::AST::ExprValueType C1::AST::DeclRefExpr::ValueType() const
{
	if (!ReturnType()->IsArrayType())
		return LValue;
	else
		return RValue;
}

bool C1::AST::DeclRefExpr::HasSideEffect() const
{
	return false;
}

bool C1::AST::DeclRefExpr::Evaluatable() const
{
	auto decl = dynamic_cast<const VariableDeclaration*>(RefDecl());
	return decl && decl->InitializeExpr()->Evaluatable();
}

DeclRefExpr* C1::AST::DeclRefExpr::MakeDeclRefExpr(DeclContext* lookup_context, TypeContext* type_context, const std::string &name)
{
	return new DeclRefExpr(lookup_context, type_context, name);
}

void C1::AST::DeclRefExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	size_t base = PCode::SS_BP;
	auto variable = dynamic_cast<VariableDeclaration*>(m_RefDecl);
	auto param = dynamic_cast<ParameterDeclaration*>(m_RefDecl);
	auto func = dynamic_cast<FunctionDeclaration*>(m_RefDecl);
	if (m_RefDecl->StorageClassSpecifier() == SCS_STATIC) base = PCode::DS;
	else if (variable && variable->IsGlobal()) base = PCode::SS;
	else if (param) base = PCode::SS_BP;
	else if (func) base = PCode::SS;
	dome.EmplaceInstruction(lit, base, m_RefDecl->Offset());
}

void C1::AST::DeclRefExpr::Generate(C1::PCode::CodeDome& dome)
{
	GenerateLValue(dome);
	dome.EmplaceInstruction(lar, 0, 0);
}

C1::AST::ComposedValue C1::AST::DeclRefExpr::Evaluate() const
{
	auto decl = dynamic_cast<const VariableDeclaration*>(RefDecl());
	if (decl && decl->DeclType().IsConst() && decl->InitializeExpr()->Evaluatable())
	{
		return decl->InitializeExpr()->Evaluate();
	}
	else
	{
		C1::AST::ComposedValue val;
		val.Int = 0;
		return val;
	}
}



C1::AST::UnaryExpr::UnaryExpr(OperatorsEnum op, Expr* sub_expr)
: m_Operator(op), m_SubExpr(sub_expr)
{
	sub_expr->SetParent(this);
	this->SetLocation(sub_expr->Location());
}

void C1::AST::UnaryExpr::Dump(std::ostream& os) const
{
	os << Operator() << *SubExpr();
}

// Basically , most unary operator returns the same type of its base
C1::AST::QualType C1::AST::UnaryExpr::ReturnType() const
{
	return m_SubExpr->ReturnType();
}

C1::AST::ExprValueType C1::AST::UnaryExpr::ValueType() const
{
	if (m_Operator != OP_ADDADD && m_Operator != OP_SUBSUB)
		return RValue;
	else
		return LValue;
}

bool C1::AST::UnaryExpr::HasSideEffect() const
{
	return false;
}

bool C1::AST::UnaryExpr::Evaluatable() const
{
	return m_SubExpr->Evaluatable();
}

UnaryExpr* C1::AST::UnaryExpr::MakeUnaryExpr(OperatorsEnum op, Expr* sub_expr)
{
	auto expr = new UnaryExpr(op, sub_expr);
	auto sub_type = sub_expr->ReturnType();
	switch (op)
	{
	case C1::OP_NEG:
	case C1::OP_POS:
		if (!sub_type->IsArithmeticType())
		{
			error(expr, "can not find overload of unary operator (+/-) which takes arguments type (<sub-expr-type>)");
		}
		break;
	case C1::OP_ADDADD:
	case C1::OP_SUBSUB:
	case C1::OP_POSFIX_ADDADD:
	case C1::OP_POSFIX_SUBSUB:
		if (!sub_type->IsIntegerType() && !sub_type->IsPointerType())
		{
			error(expr, "can not find overload of operator (++/--) which takes arguments type (<sub-expr-type>)");
		}
		if (sub_type.IsConst())
		{
			error(expr, "operand of operator (++/--) can not be an const type");
		}
		if (sub_expr->ValueType())
		{
			error(expr, "operand of operator (++/--) must be a L-Value expression");
		}
		break;
	case C1::OP_NOT:
	case C1::OP_REVERSE:
		if (!sub_type->IsInitializerListType())
		{
			error(expr, "can not find overload of unary operator (!/~) which takes arguments type (<sub-expr-type>)");
		}
		break;
	default:
		break;
	}
	return expr;
}

void C1::AST::UnaryExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	m_SubExpr->GenerateLValue(dome);
	dome.EmplaceInstruction(opr, 0, m_Operator); //unary operator don't subtract stack top
	//dome.EmplaceInstruction(isp, 0, 1); //Recover the address we just used.
	//m_SubExpr->GenerateLValue(dome);
}

C1::AST::ComposedValue C1::AST::UnaryExpr::Evaluate() const
{
	auto val = m_SubExpr->Evaluate();
	switch (m_Operator)
	{
	case C1::OP_NEG:
	{
					   auto type = m_SubExpr->ReturnType();
					   if (type->IsFloatType())
						   val.Float = -val.Float;
					   else
						   val.Int = -val.Int;
	}
		break;
	case C1::OP_NOT:
	{val.Int = !val.Int; }
		break;
	case C1::OP_REVERSE:
	{val.Int = ~val.Int; }
		break;
	case C1::OP_ADDADD:
	case C1::OP_SUBSUB:
		break;
	case C1::OP_POS:
	default:
		break;
	}
	return val;
}

void C1::AST::UnaryExpr::Generate(C1::PCode::CodeDome& dome)
{
	switch (m_Operator)
	{
	case C1::OP_NEG:
	case C1::OP_NOT:
	case C1::OP_REVERSE:
		m_SubExpr->Generate(dome);
		dome << gen(opr, 0, m_Operator);
		break;
	case C1::OP_ADDADD:
	case C1::OP_SUBSUB:
		GenerateLValue(dome);
		dome << gen(lar, 0, 0);//Read back our result.
		break;
	case C1::OP_POS:
	default:
		break;
	}
}

C1::AST::AssignExpr::AssignExpr(Expr* lhs, Expr* rhs)
: BinaryExpr(OP_ASSIGN, lhs, rhs)
{
}

C1::AST::AssignExpr::AssignExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
: BinaryExpr(op, lhs, rhs)
{
}

const C1::OperatorsEnum C1::AST::AssignExpr::PrefixedOperator() const
{
	return OperatorsEnum(m_Operator - OP_ADD_ASSIGN + OP_ADD);
}

C1::AST::QualType C1::AST::AssignExpr::ReturnType() const
{
	return LeftSubExpr()->ReturnType();
}

C1::AST::ExprValueType C1::AST::AssignExpr::ValueType() const
{
	return LValue;
	//int a;
	//(a = 3) = 5;
}

bool C1::AST::AssignExpr::HasSideEffect() const
{
	return true;
}

bool C1::AST::AssignExpr::Evaluatable() const
{
	if (IsComposed())
		return false;
	else
		return RightSubExpr()->Evaluatable();
}

Expr::ComposedValue C1::AST::AssignExpr::Evaluate() const
{
	if (IsComposed() || !RightSubExpr()->Evaluatable())
	{
		ComposedValue val;
		val.Int = 0;
		return val;
	}
	else
	{
		return RightSubExpr()->Evaluate();
	}
}

AssignExpr* C1::AST::AssignExpr::MakeAssignExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
{
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	ltype.AddConst();
	rhs = CastExpr::MakeImplictitCastExpr(ltype, rhs);
	ltype = lhs->ReturnType();
	if (op != OP_ASSIGN)
		rhs = ArithmeticExpr::MakeArithmeticExpr(GetComposedOperator(op), lhs, rhs);
	auto expr = new AssignExpr(op, lhs, rhs);
	if (lhs->ValueType() != LValue)
	{
		error(expr, "Assigned expression must have L-Value");
	}
	if (ltype.IsConst())
	{
		error(expr, "Assigned expression must have an non-const qualified type.");
	}
	return expr;
}

C1::OperatorsEnum C1::AST::AssignExpr::GetComposedOperator(OperatorsEnum op)
{
	return OperatorsEnum(op - OP_ADD_ASSIGN + OP_ADD);
}

void C1::AST::AssignExpr::Dump(std::ostream& ostr) const
{
	if (m_Operator == OP_ASSIGN)
		BinaryExpr::Dump(ostr);
	else
		ostr << *LeftSubExpr() << Operator() << *dynamic_cast<const BinaryExpr*>(RightSubExpr())->RightSubExpr();
}

void C1::AST::AssignExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	RightSubExpr()->Generate(dome);
	LeftSubExpr()->GenerateLValue(dome);
	dome << gen(sar, 0, 0);
	auto size = RightSubExpr()->ReturnType()->Size();
	dome << gen(isp, 0, -static_cast<int>(size));
	LeftSubExpr()->GenerateLValue(dome);
	// It's stupid , but I can't figure out a more elegant way
}

void C1::AST::AssignExpr::Generate(C1::PCode::CodeDome& dome)
{
	RightSubExpr()->Generate(dome);
	LeftSubExpr()->GenerateLValue(dome);
	dome << gen(sar, 0, 0);
}

C1::AST::IndexExpr::IndexExpr(Expr* host_expr, Expr* index_expr)
: BinaryExpr(OP_INDEX, host_expr, index_expr)
{
}

void C1::AST::IndexExpr::Dump(std::ostream& os) const
{
	os << *Host() << '[' << *Index() << ']';
}

C1::AST::QualType C1::AST::IndexExpr::ReturnType() const
{
	return LeftSubExpr()->ReturnType().As<DereferencableType>()->Base();
}

C1::AST::ExprValueType C1::AST::IndexExpr::ValueType() const
{
	return LValue;
}

Expr::ComposedValue C1::AST::IndexExpr::Evaluate() const
{
	Expr::ComposedValue val;
	val.Int = 0;
	return val;
}

bool C1::AST::IndexExpr::Evaluatable() const
{
	return false;
}

IndexExpr* C1::AST::IndexExpr::MakeIndexExpr(Expr* lhs, Expr* rhs)
{
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	auto expr = new IndexExpr(lhs, rhs);
	if (!ltype->IsAddressType())
	{
		error(expr, "Left operand of Index operator must have a dereference able type");
	}
	if (!rtype->IsIntegerType())
	{
		error(expr, "Right operand of Index operator must have a integer type");
	}
	return expr;
}

void C1::AST::IndexExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	LeftSubExpr()->GenerateLValue(dome);
	dome << *RightSubExpr();
	auto size = LeftSubExpr()->ReturnType().As<DereferencableType>()->Base()->Size();
	if (size != 1)
	{
		dome << gen(lit, 0, size);
		dome << gen(opr, 0, OP_MUL);
	}
	dome << gen(opr, 0, OP_ADD);
}

void C1::AST::IndexExpr::Generate(C1::PCode::CodeDome& dome)
{
	GenerateLValue(dome);
	dome << gen(lar, 0, 0);
}

C1::AST::ArithmeticExpr::ArithmeticExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
: BinaryExpr(op, lhs, rhs)
{
	//IsArithmeticOperator(op);
}

ArithmeticExpr* C1::AST::ArithmeticExpr::MakeArithmeticExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
{
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	// Well , you always need to build it.

	if (!ltype->IsArithmeticType() || !rtype->IsArithmeticType())
	{
		auto expr = new ArithmeticExpr(op, lhs, rhs);
		// C-style pointer arithmetic
		if (op == OP_ADD)
		{
			// int + pointer || pointer + int
			if (ltype->IsAddressType() && rtype->IsIntegerType() || ltype->IsIntegerType() && rtype->IsAddressType())
				return expr;
		}
		else if (op == OP_SUB)
		{
			// pointer - int || pointer(base) - pointer(base)
			if (ltype->IsAddressType() && rtype->IsIntegerType())
				return expr;
			else if (ltype->IsAddressType() && rtype->IsAddressType())
			if (ltype.As<DereferencableType>()->Base() == rtype.As<DereferencableType>()->Base())
			{
				return expr;
			}
		}
		else if (op == OP_EQL || op == OP_NEQ)
		{
			// Only support address type equal/not-equal comparison
			if (ltype->IsAddressType() && rtype->IsAddressType() && ltype.As<DereferencableType>()->Base() == rtype.As<DereferencableType>()->Base())
				return expr;
		}
		error(expr, "no overload for operator <op> with argument type (<lhs-type> , <rhs-type>)");
		return expr;
	}
	else {
		switch (op)
		{
			// Integer & Float Type Operators
		case C1::OP_ADD:
		case C1::OP_SUB:
		case C1::OP_MUL:
		case C1::OP_DIV:

		case C1::OP_EQL:
		case C1::OP_NEQ:
		case C1::OP_LSS:
		case C1::OP_GEQ:
		case C1::OP_GTR:
		case C1::OP_LEQ:
		{
						   auto generic_type = get_most_generic_arithmetic_type(ltype, rtype);
						   lhs = CastExpr::MakeImplictitCastExpr(generic_type, lhs);
						   rhs = CastExpr::MakeImplictitCastExpr(generic_type, rhs);
						   return new ArithmeticExpr(op, lhs, rhs);
		}
			break;
			// Integer Type Operators
		case C1::OP_MOD:
		case C1::OP_AND:
		case C1::OP_OR:
		case C1::OP_XOR:
		case C1::OP_LSH:
		case C1::OP_RSH:
		{
						   auto expr = new ArithmeticExpr(op, lhs, rhs);
						   if (!ltype->IsIntegerType() || !rtype->IsIntegerType())
						   {
							   error(expr, "no overload for operator <op> with argument type (<lhs-type> , <rhs-type>)");
							   return expr;
						   }
						   return expr;
		}
			break;
		default:
			return new ArithmeticExpr(op, lhs, rhs);
			break;
		}
	}
}

C1::AST::QualType C1::AST::ArithmeticExpr::ReturnType() const
{
	auto lhs = LeftSubExpr();
	auto rhs = RightSubExpr();
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	auto op = m_Operator;
	if (ltype == rtype)
		return ltype;
	if (ltype->IsErrorType()) return ltype;
	if (rtype->IsErrorType()) return rtype;

	if (!ltype->IsArithmeticType() || !rtype->IsArithmeticType())
	{
		// C-style pointer arithmetic
		if (op == OP_ADD)
		{
			// int + pointer || pointer + int
			if (ltype->IsAddressType() && rtype->IsIntegerType())
				return MakeConst(ltype.As<DereferencableType>()->GetPointerType());
			else if (ltype->IsIntegerType() && rtype->IsAddressType())
				return MakeConst(rtype.As<DereferencableType>()->GetPointerType());
		}
		else if (op == OP_SUB)
		{
			// pointer - int || pointer(base) - pointer(base)
			if (ltype->IsAddressType() && rtype->IsIntegerType())
				return MakeConst(ltype.As<DereferencableType>()->GetPointerType());
			else if (ltype->IsAddressType() && rtype->IsAddressType() && (ltype.As<DereferencableType>()->Base() == rtype.As<DereferencableType>()->Base()))
				return ltype->AffiliatedContext()->Int();
			else // Int - Pointer or Different pointer minus
				return ltype->AffiliatedContext()->Error();
		}
		else if (op == OP_EQL || op == OP_NEQ)
		{
			// Only support address type equal/not-equal comparison
			if (ltype->IsAddressType() && rtype->IsAddressType() && ltype.As<DereferencableType>()->Base() == rtype.As<DereferencableType>()->Base())
				return ltype->AffiliatedContext()->Bool();
			else
				return ltype->AffiliatedContext()->Bool();
			// It's part of the recovery
			//return ltype->AffiliatedContext()->Error();
		}
		//error(expr, "no overload for operator <op> with argument type (<lhs-type> , <rhs-type>)");
	}
	else
	{
		switch (op)
		{
			// Integer & Float Type Operators
		case C1::OP_EQL:
		case C1::OP_NEQ:
		case C1::OP_LSS:
		case C1::OP_GEQ:
		case C1::OP_GTR:
		case C1::OP_LEQ:
		{
						   return ltype->AffiliatedContext()->Bool(); // assume ltye==rtype
		}
			break;
			// Integer Type Operators
		case C1::OP_ADD:
		case C1::OP_SUB:
		case C1::OP_MUL:
		case C1::OP_DIV:
		case C1::OP_MOD:
		case C1::OP_AND:
		case C1::OP_OR:
		case C1::OP_XOR:
		case C1::OP_LSH:
		case C1::OP_RSH:
		{
						   return ltype; // assume ltye==rtype
		}
			break;
		default:
			break;
		}
	}
	return ltype->AffiliatedContext()->Error();
}

void C1::AST::ArithmeticExpr::Generate(C1::PCode::CodeDome& dome)
{
	auto lhs = LeftSubExpr();
	auto rhs = RightSubExpr();
	auto op = m_Operator;
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	// Well , you always need to build it.

	if (!ltype->IsArithmeticType() || !rtype->IsArithmeticType())
	{
		if (op == OP_ADD)
		{
			// int + pointer || pointer + int
			if (ltype->IsAddressType() && rtype->IsIntegerType())
			{
				dome << *lhs;
				dome << *rhs;
				auto size = ltype.As<DereferencableType>()->Base()->Size();
				if (size != 1)
				{
					dome << gen(lit, 0, size);
					dome << gen(opr, 0, OP_MUL);
				}
				dome << gen(opr, 0, OP_ADD);
			}
			if (ltype->IsIntegerType() && rtype->IsAddressType())
			{
				dome << *lhs;
				auto size = ltype.As<DereferencableType>()->Base()->Size();
				if (size != 1)
				{
					dome << gen(lit, 0, size);
					dome << gen(opr, 0, OP_MUL);
				}
				dome << *rhs;
				dome << gen(opr, 0, OP_ADD);
			}
		}
		else if (op == OP_SUB)
		{
			// pointer - int || pointer(base) - pointer(base)
			if (ltype->IsAddressType() && rtype->IsIntegerType())
			{
				dome << *lhs;
				dome << *rhs;
				auto size = ltype.As<DereferencableType>()->Base()->Size();
				if (size != 1)
				{
					dome << gen(lit, 0, size);
					dome << gen(opr, 0, OP_MUL);
				}
				dome << gen(opr, 0, OP_SUB);
			}
			else if (ltype->IsAddressType() && rtype->IsAddressType())
			if (ltype.As<DereferencableType>()->Base() == rtype.As<DereferencableType>()->Base())
			{
				auto size = ltype.As<DereferencableType>()->Base()->Size();
				dome << *lhs;
				dome << *rhs;
				dome << gen(opr, 0, OP_SUB);
				if (size != 1)
				{
					dome << gen(lit, 0, size);
					dome << gen(opr, 0, OP_DIV);
				}
			}
		}
		else if (op == OP_EQL || op == OP_NEQ)
		{
			dome << *lhs;
			dome << *rhs;
			dome << gen(opr, 0, op);
		}
	}
	else {
		switch (op)
		{
			// Integer & Float Type Operators
		case C1::OP_ADD:
		case C1::OP_SUB:
		case C1::OP_MUL:
		case C1::OP_DIV:
		case C1::OP_EQL:
		case C1::OP_NEQ:
		case C1::OP_LSS:
		case C1::OP_GEQ:
		case C1::OP_GTR:
		case C1::OP_LEQ:
		{
						   long ir_type = 0;
						   if (ltype->IsFloatType()) ir_type = 3;

						   dome << *lhs;
						   dome << *rhs;
						   dome << gen(opr, ir_type, op);
		}
			break;
			// Integer Type Operators
		case C1::OP_MOD:
		case C1::OP_AND:
		case C1::OP_OR:
		case C1::OP_XOR:
		case C1::OP_LSH:
		case C1::OP_RSH:
		{
						   dome << *lhs;
						   dome << *rhs;
						   dome << gen(opr, 0, op);
		}
			break;
		default:
			break;
		}
	}
}

C1::AST::LogicExpr::LogicExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
: BinaryExpr(op, lhs, rhs)
{
}

LogicExpr* C1::AST::LogicExpr::MakeLogicExpr(OperatorsEnum op, Expr* lhs, Expr* rhs)
{
	auto ltype = lhs->ReturnType();
	auto rtype = rhs->ReturnType();
	auto expr = new LogicExpr(op, lhs, rhs);
	if (!ltype->IsIntegerType() || !rtype->IsIntegerType())
	{
		error(expr, "no overload for operator <logic-op> with argument type (<lhs-type> , <rhs-type>)");
	}
	return expr;
}

C1::AST::QualType C1::AST::LogicExpr::ReturnType() const
{
	return LeftSubExpr()->ReturnType()->AffiliatedContext()->Bool();
}

void C1::AST::LogicExpr::Generate(C1::PCode::CodeDome& dome)
{
	if (m_Operator == OP_OROR)
	{
		dome << *LeftSubExpr();
		int cx = dome.EmplaceInstruction(jpe, 1, 0);
		dome << *RightSubExpr();
		dome << gen(opr, 0, m_Operator);//op is the key!
		dome.Instruction(cx).a = dome.InstructionCount();
	}
	else if (m_Operator == OP_ANDAND)
	{
		dome << *LeftSubExpr();
		int cx = dome.EmplaceInstruction(jpc, 1, 0);
		dome << *RightSubExpr();
		dome << gen(opr, 0, m_Operator);//op is the key!
		dome.Instruction(cx).a = dome.InstructionCount();
	}
}

C1::AST::ConditionalExpr::ConditionalExpr(Expr* condition, Expr* true_expr, Expr* false_expr)
: m_Condition(condition), m_TrueExpr(true_expr), m_FalseExpr(false_expr)
{}

void C1::AST::ConditionalExpr::Dump(std::ostream& os) const
{
	os << *m_Condition << " ? " << *m_TrueExpr << " : " << *m_FalseExpr;
	//bool boo = true;
	//auto c = boo ? "ac" : 1.5f;
}

C1::AST::QualType C1::AST::ConditionalExpr::ReturnType() const
{
	if (m_TrueExpr->ReturnType() == m_FalseExpr->ReturnType())
	{
		return m_TrueExpr->ReturnType();
	}
	else return QualType(nullptr); // Should throw here
}

C1::AST::ExprValueType C1::AST::ConditionalExpr::ValueType() const
{
	return RValue;
}

bool C1::AST::ConditionalExpr::HasSideEffect() const
{
	return m_Condition->HasSideEffect() || m_TrueExpr->HasSideEffect() || m_FalseExpr->HasSideEffect();
}

ComposedValue C1::AST::ConditionalExpr::Evaluate() const
{
	assert(m_Condition->Evaluatable());
	auto cond = m_Condition->Evaluate();
	if (cond.Bool)
		return m_TrueExpr->Evaluate();
	else
		return m_FalseExpr->Evaluate();
}

bool C1::AST::ConditionalExpr::Evaluatable() const
{
	if (!m_Condition->Evaluatable()) return false;
	auto cond = m_Condition->Evaluate();
	if (cond.Bool)
		return m_TrueExpr->Evaluatable();
	else
		return m_FalseExpr->Evaluatable();
}


C1::AST::TypeExpr::TypeExpr(QualifiedTypeSpecifier* qual_type_specifier, Declarator* declarator)
: m_Specifier(qual_type_specifier), m_Declarator(declarator)
{
	m_Specifier->SetParent(this);

	m_Declarator->SetParent(this);
	SetLocation(m_Specifier->Location() + m_Declarator->Location());
	m_Type = m_Declarator->DecorateType(m_Specifier->RepresentType());
}

void C1::AST::TypeExpr::Dump(std::ostream& os) const
{
	os << *m_Specifier << *m_Declarator;
}

void C1::AST::TypeExpr::Generate(C1::PCode::CodeDome& dome)
{
}

C1::AST::MemberExpr::MemberExpr(Expr* host, const std::string &member_name, OperatorsEnum op)
: m_HostExpr(host), m_MemberName(member_name), m_Operator(op)
{
	host->SetParent(this);
	QualType hostType = host->ReturnType();
	hostType = remove_alias(hostType);
	if (op == OP_ARROW)
	{
		if (hostType->IsPointerType())
		{
			hostType = hostType.As<DereferencableType>()->Base();
		}
		else
		{
			error(this, "Left operand of operator -> must have pointer types.");
		}
	}
	if (!hostType->IsStructType())
	{
		error(this, "Left operand of member operator must have record types.");
	}
	else
	{
		auto def = hostType.As<StructType>()->Members();
		auto decl = def->lookup_local(m_MemberName);
		m_MemberDeclaration = dynamic_cast<FieldDeclaration*>(decl);
		if (!decl)
		{
			//char[100] buf;
			//sprintf(buf, "%s is not a member of type %s", m_MemberName, host->ReturnType()->ToString());
			error(this, "Right operand is not an member of left operand.");
		}
	}
}

bool C1::AST::MemberExpr::IsDirect() const
{
	return m_Operator == OP_DOT;
}

void C1::AST::MemberExpr::Dump(std::ostream& os) const
{
	os << *Host() << m_Operator << m_MemberName;
}

C1::AST::QualType C1::AST::MemberExpr::ReturnType() const
{
	return m_MemberDeclaration->DeclType();
}

C1::AST::ExprValueType C1::AST::MemberExpr::ValueType() const
{
	return LValue;
}

bool C1::AST::MemberExpr::HasSideEffect() const
{
	return false;
}

bool C1::AST::MemberExpr::Evaluatable() const
{
	return false;
}

MemberExpr* C1::AST::MemberExpr::MakeMemberExpr(Expr* host, const std::string &member_name, OperatorsEnum op)
{
	return new MemberExpr(host, member_name, op);
}

void C1::AST::MemberExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	if (IsDirect())
		m_HostExpr->GenerateLValue(dome);
	else
		m_HostExpr->Generate(dome);
	dome << gen(lit, 0, m_MemberDeclaration->Offset());
	dome << gen(opr, 0, OP_ADD);
}

void C1::AST::MemberExpr::Generate(C1::PCode::CodeDome& dome)
{
	GenerateLValue(dome);
	dome << gen(lar, 0, 0);
}

C1::AST::ExplicitCastExpr::ExplicitCastExpr(TypeExpr* target_type_expr, Expr* source_expr)
: m_TargetTypeExpr(target_type_expr), CastExpr(source_expr, target_type_expr->DeclType())
{
}

void C1::AST::ExplicitCastExpr::Dump(std::ostream& os) const
{
	os << "cast<" << *m_TargetTypeExpr << ">(" << *m_SourceExpr << ")";
}

C1::AST::StringLiteral::StringLiteral(TypeContext* type_context, const char* raw_str)
: ConstantLiteral(type_context, raw_str)
{
	m_DecodedString = DecodeString(m_RawLiteral);
	m_Value.String = m_DecodedString.data();
}

C1::AST::QualType C1::AST::StringLiteral::ReturnType() const
{
	return MakeConst(m_TypeContext->String());
}

std::string C1::AST::StringLiteral::DecodeString(const std::string & raw_str)
{
	int n = raw_str.length() - 2;
	char buffer[256];
	int i, m = 0;
	for (i = 1; i <= n; i++)
	{
		if (raw_str[i] != '\\') buffer[m++] = raw_str[i];
		else
		{
			i++;
			switch (raw_str[i])
			{
			case 'v':
				buffer[m++] = '\v'; break;
			case 'f':
				buffer[m++] = '\f'; break;
			case 'r':
				buffer[m++] = '\r'; break;
			case 't':
				buffer[m++] = '\t'; break;
			case 'n':
				buffer[m++] = '\n'; break;
			case 'b':
				buffer[m++] = '\b'; break;
			default:
				break;
			}
		}
	}//The cpmlpex string support (allowed character like \n)
	//	     strncpy(buffer,yytext+1,n);
	buffer[m] = '\0';
	return buffer;
	//yylval.val.type = ConstString;
	//yylval.val.Int = smp;
	//smp += m / 4 + 1;
	////	     smp+=n+1;
	////	     if (smp%4) smp+=(4-smp%4);  //for the alline
	////	     printf("Warning : String supporting is not finished yet >_<, use this may cause some unwanted result.\n");
	//return(NUMBER);
}

std::string C1::AST::StringLiteral::EncodeString(const std::string & raw_str)
{
	return raw_str;
}

void C1::AST::StringLiteral::Generate(C1::PCode::CodeDome& dome)
{
	size_t addr = dome.EmplaceString(m_DecodedString);
	dome << gen(lit, PCode::DS, addr); // In static data segment
}

void C1::AST::InitializerList::Dump(std::ostream& os) const
{
	if (empty())
	{
		os << "{}";
		return;
	}
	os << "{";
	for (auto initializer : *this)
	{
		os << *initializer << ", ";
	}
	os << "\b\b}";
}

C1::AST::QualType C1::AST::InitializerList::ReturnType() const
{
	if (m_ReturnType.get())
		return m_ReturnType;
	InitializerListType* type = m_TypeContext->NewInitializerListType();
	const_cast<QualType&>(m_ReturnType) = QualType(type);
	for (auto expr : *this)
	{
		type->ElementTypes().push_back(expr->ReturnType());
	}
	return m_ReturnType;
}

C1::AST::ExprValueType C1::AST::InitializerList::ValueType() const
{
	return RValue;
}


bool C1::AST::InitializerList::HasSideEffect() const
{
	for (auto initailizer : *this)
	{
		if (initailizer->HasSideEffect())
			return true;
	}
	return false;
}

C1::AST::Expr::ComposedValue C1::AST::InitializerList::Evaluate() const
{
	Expr::ComposedValue val;
	val.ValueList = new std::vector<ComposedValue>(size());
	auto &valueList = *val.ValueList;
	for (auto expr : *this)
	{
		assert(expr->Evaluatable());
		valueList.emplace_back(expr->Evaluate());
	}
	return val;
}

bool C1::AST::InitializerList::Evaluatable() const
{
	for (auto initailizer : *this)
	{
		if (initailizer->Evaluatable())
			return true;
	}
	return false;
}

C1::AST::InitializerList::InitializerList(TypeContext *context)
: m_TypeContext(context)
{
}

void C1::AST::InitializerList::Generate(C1::PCode::CodeDome& dome)
{
	for (auto var : *this)
	{
		dome << *var;
	}
}

C1::AST::CommaExpr::CommaExpr(Expr* lhs, Expr* rhs)
: BinaryExpr(OP_COMMA, lhs, rhs)
{

}

void C1::AST::CommaExpr::Generate(C1::PCode::CodeDome& dome)
{
	dome << *LeftSubExpr();
	dome << gen(isp, 0, -static_cast<int>(LeftSubExpr()->ReturnType()->Size()));
	dome << *RightSubExpr();
}

C1::AST::QualType C1::AST::CommaExpr::ReturnType() const
{
	return RightSubExpr()->ReturnType();
}

C1::AST::ComposedValue C1::AST::CommaExpr::Evaluate() const
{
	return RightSubExpr()->Evaluate();
}

bool C1::AST::CommaExpr::Evaluatable() const
{
	return RightSubExpr()->Evaluatable();
}



QualType C1::AST::ParenExpr::ReturnType() const
{
	return m_Base->ReturnType();
}

C1::AST::ExprValueType C1::AST::ParenExpr::ValueType() const
{
	return m_Base->ValueType();
}

bool C1::AST::ParenExpr::HasSideEffect() const
{
	return m_Base->HasSideEffect();
}

bool C1::AST::ParenExpr::Evaluatable() const
{
	return m_Base->Evaluatable();
}

C1::AST::ParenExpr::ParenExpr(Expr* base)
: m_Base(base)
{

}

void C1::AST::ParenExpr::Dump(std::ostream& os) const
{
	os << "(" << *m_Base << ")";
}

void C1::AST::ParenExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	m_Base->GenerateLValue(dome);
}

C1::AST::ComposedValue C1::AST::ParenExpr::Evaluate() const
{
	return m_Base->Evaluate();
}

void C1::AST::ParenExpr::Generate(C1::PCode::CodeDome& dome)
{
	m_Base->Generate(dome);
}

C1::AST::ExprValueType C1::AST::DereferenceExpr::ValueType() const
{
	return LValue;
}

bool C1::AST::DereferenceExpr::Evaluatable() const
{
	return false;
}

C1::AST::QualType C1::AST::DereferenceExpr::ReturnType() const
{
	auto def_type = SubExpr()->ReturnType().As<PointerType>();
	if (def_type)
		return def_type->Base();
	else
	{
		return SubExpr()->ReturnType()->AffiliatedContext()->Error();
	}
}

C1::AST::DereferenceExpr::DereferenceExpr(Expr* base)
: UnaryExpr(OP_DEREF, base)
{

}

DereferenceExpr* C1::AST::DereferenceExpr::MakeDereferenceExpr(Expr* base)
{
	auto expr = new DereferenceExpr(base);
	if (!base->ReturnType()->IsAddressType())
	{
		error(expr, "operand of Address operator (*) must be dereference-able.");
	}
	return expr;
}

void C1::AST::DereferenceExpr::GenerateLValue(C1::PCode::CodeDome& dome)
{
	dome << *m_SubExpr;
}

void C1::AST::DereferenceExpr::Generate(C1::PCode::CodeDome& dome)
{
	GenerateLValue(dome);
	dome << gen(lar, 0, 0);
}

C1::AST::ExprValueType C1::AST::AddressOfExpr::ValueType() const
{
	return RValue;
}

bool C1::AST::AddressOfExpr::Evaluatable() const
{
	return false;
}

C1::AST::QualType C1::AST::AddressOfExpr::ReturnType() const
{
	return MakeConst(MakePointer(SubExpr()->ReturnType()));
}

C1::AST::AddressOfExpr::AddressOfExpr(Expr* base) : UnaryExpr(OP_ADDRESS, base)
{

}

AddressOfExpr* C1::AST::AddressOfExpr::MakeAddressOfExpr(Expr* base)
{
	auto expr = new AddressOfExpr(base);
	if (base->ValueType() != LValue)
	{
		error(expr, "operand of Address operator (&) must be an L-Value expression.");
	}
	return expr;
}

void C1::AST::AddressOfExpr::Generate(C1::PCode::CodeDome& dome)
{
	m_SubExpr->GenerateLValue(dome);
}

C1::AST::ExprValueType C1::AST::IncrementExpr::ValueType() const
{
	return LValue;
}

bool C1::AST::IncrementExpr::HasSideEffect() const
{
	return true;
}

bool C1::AST::IncrementExpr::Evaluatable() const
{
	return false;
}

C1::AST::QualType C1::AST::SizeofExpr::ReturnType() const
{
	return m_SubExpr->ReturnType()->AffiliatedContext()->Int();
}

C1::AST::ExprValueType C1::AST::SizeofExpr::ValueType() const
{
	return RValue;
}

bool C1::AST::SizeofExpr::Evaluatable() const
{
	return true;
}

C1::AST::Expr::ComposedValue C1::AST::SizeofExpr::Evaluate() const
{
	ComposedValue val;
	val.Int = m_SubExpr->ReturnType()->Size();
	return val;
}

C1::AST::SizeofExpr::SizeofExpr(Expr* expr)
: UnaryExpr(OP_SIZEOF, expr)
{

}

void C1::AST::SizeofExpr::Generate(C1::PCode::CodeDome& dome)
{
	auto size = m_SubExpr->ReturnType()->Size();
	dome << gen(lit, 0, size);
}

C1::AST::QualType C1::AST::SizeofTypeExpr::ReturnType() const
{
	return m_TypeExpr->DeclType()->AffiliatedContext()->Int();
}

C1::AST::ExprValueType C1::AST::SizeofTypeExpr::ValueType() const
{
	return RValue;
}

bool C1::AST::SizeofTypeExpr::HasSideEffect() const
{
	return false;
}

C1::AST::Expr::ComposedValue C1::AST::SizeofTypeExpr::Evaluate() const
{
	ComposedValue val;
	val.Int = m_TypeExpr->DeclType()->Size();
	return val;
}

bool C1::AST::SizeofTypeExpr::Evaluatable() const
{
	return true;
}

C1::AST::SizeofTypeExpr::SizeofTypeExpr(TypeExpr* type_expr)
: m_TypeExpr(type_expr)
{
	type_expr->SetParent(this);
}

void C1::AST::SizeofTypeExpr::Dump(std::ostream& os) const
{
	os << "sizeof (" << *m_TypeExpr << ")";
}

void C1::AST::SizeofTypeExpr::Generate(C1::PCode::CodeDome& dome)
{
	dome << gen(lit, 0, m_TypeExpr->DeclType()->Size());
}

C1::AST::ExprValueType C1::AST::ConstantLiteral::ValueType() const
{
	return RValue;
}

bool C1::AST::ConstantLiteral::HasSideEffect() const
{
	return false;
}

C1::AST::Expr::ComposedValue C1::AST::ConstantLiteral::Evaluate() const
{
	return m_Value;
}

bool C1::AST::ConstantLiteral::Evaluatable() const
{
	return true;
}

void C1::AST::ConstantLiteral::Dump(std::ostream& ostr) const
{
	ostr << m_RawLiteral;
}

C1::AST::ConstantLiteral::ConstantLiteral(TypeContext* type_context, const std::string& raw_str)
: m_TypeContext(type_context), m_RawLiteral(raw_str)
{
}

C1::AST::BooleanLiteral::BooleanLiteral(TypeContext* type_context, const char* raw_str, bool value)
	: ConstantLiteral(type_context, raw_str)
{
	m_Value.Bool = value;
}

C1::AST::QualType C1::AST::BooleanLiteral::ReturnType() const
{
	return MakeConst(m_TypeContext->Bool());
}

void C1::AST::BooleanLiteral::Generate(C1::PCode::CodeDome& dome)
{
	dome << gen(lit, 0, m_Value.Int);
}

C1::AST::IntegerLiteral::IntegerLiteral(TypeContext* type_context, const char* raw_str, int value, int dec)
: ConstantLiteral(type_context, raw_str), m_decimal(dec)
{
	m_Value.Int = value;
}

C1::AST::QualType C1::AST::IntegerLiteral::ReturnType() const
{
	return MakeConst(m_TypeContext->Int());
}

void C1::AST::IntegerLiteral::Generate(C1::PCode::CodeDome& dome)
{
	dome << gen(lit, 0, m_Value.Int);
}

C1::AST::FloatLiteral::FloatLiteral(TypeContext* type_context, const char* raw_str, float value)
: ConstantLiteral(type_context, raw_str)
{
	m_Value.Float = value;
}

C1::AST::QualType C1::AST::FloatLiteral::ReturnType() const
{
	return MakeConst(m_TypeContext->Float());
}

void C1::AST::FloatLiteral::Generate(C1::PCode::CodeDome& dome)
{
	size_t addr = dome.EmplaceFloat(m_Value.Float);
	dome << gen(lod, PCode::DS, addr);
}

C1::AST::CharacterLiteral::CharacterLiteral(TypeContext* type_context, const char* raw_str, char value)
: ConstantLiteral(type_context, raw_str)
{
	m_Value.Char = value;
}

C1::AST::QualType C1::AST::CharacterLiteral::ReturnType() const
{
	return MakeConst(m_TypeContext->Char());
}

void C1::AST::CharacterLiteral::Generate(C1::PCode::CodeDome& dome)
{
	dome << gen(lit, 0, m_Value.Char);
}

C1::AST::QualType C1::AST::AtomInitializer::ReturnType() const
{
	return m_ValueExpr->ReturnType();
}

C1::AST::ExprValueType C1::AST::AtomInitializer::ValueType() const
{
	return m_ValueExpr->ValueType();
}

bool C1::AST::AtomInitializer::HasSideEffect() const
{
	return m_ValueExpr->HasSideEffect();
}

C1::AST::Expr::ComposedValue C1::AST::AtomInitializer::Evaluate() const
{
	return m_ValueExpr->Evaluate();
}

bool C1::AST::AtomInitializer::Evaluatable() const
{
	return m_ValueExpr->Evaluatable();
}

void C1::AST::AtomInitializer::Dump(std::ostream& os) const
{
	os << *m_ValueExpr;
}

C1::AST::AtomInitializer::AtomInitializer(Expr* value_expr)
: m_ValueExpr(value_expr)
{

}

void C1::AST::AtomInitializer::Generate(C1::PCode::CodeDome& dome)
{
	dome << *m_ValueExpr;
}

C1::AST::QualType C1::AST::CastExpr::ReturnType() const
{
	return m_TargetType;
}

C1::AST::ExprValueType C1::AST::CastExpr::ValueType() const
{
	return RValue;
}

bool C1::AST::CastExpr::HasSideEffect() const
{
	return m_SourceExpr->HasSideEffect();
}

Expr::ComposedValue C1::AST::CastExpr::Evaluate() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool C1::AST::CastExpr::Evaluatable() const
{
	return m_SourceExpr->Evaluatable();
}

void C1::AST::CastExpr::Dump(std::ostream& ostr) const
{
	ostr << *m_SourceExpr;
}

C1::AST::CastExpr::CastExpr(Expr* source_expr, QualType target_type, FunctionDeclaration* conversion_func /*= nullptr*/)
: m_SourceExpr(source_expr), m_TargetType(target_type), m_ConversionFunction(conversion_func)
{
	source_expr->SetParent(this);
	SetLocation(source_expr->Location());
}

Expr* C1::AST::CastExpr::MakeImplictitCastExpr(QualType target_type, Expr* source_expr)
{
	auto expr_type = source_expr->ReturnType();
	if (target_type.IsConst())
		expr_type.AddConst(); // non-const to const cast, perfect fine
	if (expr_type->IsErrorType() || target_type->IsErrorType())
		return source_expr;
	if (expr_type == target_type)
		return source_expr; // source type matches target type
	else
		return new ImplicitCastExpr(target_type, source_expr);
}

void C1::AST::CastExpr::Generate(C1::PCode::CodeDome& dome)
{
	dome << *m_SourceExpr;
	auto source_type = m_SourceExpr->ReturnType();
	auto target_type = m_TargetType;
	if (source_type->IsIntegerType() && target_type->IsFloatType())
	{
		dome << gen(opr, 1, OP_CAST);
	}
	if (source_type->IsFloatType() && target_type->IsIntegerType())
	{
		dome << gen(opr, 0, OP_CAST);
	}
	// Other kind of cast is for type safety (Let's forgot the conversion function)
}

void C1::AST::ImplicitCastExpr::Dump(std::ostream& os) const
{
	os << "cast<" << m_TargetType << ">(" << *m_SourceExpr << ")";
}

C1::AST::ImplicitCastExpr::ImplicitCastExpr(QualType target_type, Expr* source_expr)
: CastExpr(source_expr, target_type)
{
}