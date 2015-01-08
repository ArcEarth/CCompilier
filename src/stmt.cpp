#include "stdafx.h"
#include "ast.h"
#include "pcode.h"
using namespace C1;
using namespace C1::AST;
using namespace std;
using namespace C1::PCode;


C1::AST::CompoundStmt::CompoundStmt()
{

}

void C1::AST::CompoundStmt::Dump(std::ostream& os) const
{
	os << "{" << endl;
	for (auto stmt : Children())
	{
		os << *stmt;
	}
	os << "}" << endl;
}

void C1::AST::CompoundStmt::Generate(C1::PCode::CodeDome& dome)
{
	size_t local_var_size = 0;
	//const size_t control_layer_size = 2;
	for (auto decl : *this)
	{
		auto var = dynamic_cast<VariableDeclaration*>(decl);
		if (var)
		{
			if (var->StorageClassSpecifier() == SCS_STATIC) {
				auto addr = dome.EmplaceDataBlcok(var->DeclType()->Size());
				var->SetOffset(addr);
			}
			else
			{
				var->SetOffset(dome.SP);
				auto varsize = var->DeclType()->Size();
				local_var_size += varsize;
				dome.SP += varsize;
			}
		}
	}
	for (auto node : Children())
	{
		dome << *node;
	}
	dome << gen(isp, 0, -static_cast<int>(local_var_size));
}

C1::AST::WhileStmt::WhileStmt(Expr* condition, Stmt* action)
: IterationStmt(action, condition)
{
}

void C1::AST::WhileStmt::Dump(std::ostream& os) const
{
	os << "while (" << *m_Condition << ")" << endl;
	os << *m_Action;
}

void C1::AST::WhileStmt::Generate(C1::PCode::CodeDome& dome)
{
	m_EntryAddress = dome.InstructionCount();
	dome << *m_Condition;
	int cx = dome.EmplaceInstruction(jpc, 0, 0);
	m_ActionEntryAddress = dome.InstructionCount();
	dome << *m_Action;
	dome << gen(jmp, 0, m_EntryAddress);
	m_ExitAddress = dome.InstructionCount();
	dome.Instruction(cx).a = m_ExitAddress;
}

C1::AST::ForStmt::ForStmt(Stmt* initialize_stmt, Expr* condition, Expr* post_action, Stmt* action)
: IterationStmt(action, condition), m_Initializer(initialize_stmt), m_PostAction(new ExprStmt(post_action))
{
	m_Initializer->SetParent(this);
	m_PostAction->SetParent(this);
}

void C1::AST::ForStmt::Dump(std::ostream& os) const
{
	os << "for (" << *m_Initializer << *m_Condition << ";" << *m_PostAction->Expression() << ")" << endl;
	os << *m_Action;
}

C1::AST::DoWhileStmt::DoWhileStmt(Stmt* action, Expr* condition)
: IterationStmt(action, condition)
{
}

void C1::AST::DoWhileStmt::Dump(std::ostream& os) const
{
	os << "do" << endl;
	os << *m_Action;
	os << "while (" << *m_Condition << ")" << endl;
}

C1::AST::IfStmt::IfStmt(Expr* condition, Stmt* then_action, Stmt* else_action /*= nullptr*/)
: m_Condition(condition), m_ThenAction(then_action), m_ElseAction(else_action)
{
	m_Condition->SetParent(this);
	m_ThenAction->SetParent(this);
	if (m_ElseAction)
		m_ElseAction->SetParent(this);
}

void C1::AST::IfStmt::Dump(std::ostream& os) const
{
	os << "if (" << *m_Condition << ")" << endl << *m_ThenAction;
	if (m_ElseAction)
		os << "else" << endl << *m_ElseAction;
}

void C1::AST::IfStmt::Generate(C1::PCode::CodeDome& dome)
{

	dome << *m_Condition;
	auto cx = dome.EmplaceInstruction(jpc, 0, 0);
	dome << *m_ThenAction;
	if (m_ElseAction)
	{
		auto cx1 = dome.EmplaceInstruction(jmp, 0, 0);
		dome.Instruction(cx).a = dome.InstructionCount();
		dome << *m_ElseAction;
		dome.Instruction(cx1).a = dome.InstructionCount();
	}
	else
	{
		dome.Instruction(cx).a = dome.InstructionCount();
	}
}


C1::AST::ExprStmt::ExprStmt(Expr* expr)
: m_Expr(expr)
{
	expr->SetParent(this);
}

void C1::AST::ExprStmt::Dump(std::ostream& os) const
{
	os << *m_Expr << ";" << endl;
}

void C1::AST::ExprStmt::Generate(C1::PCode::CodeDome& dome)
{
	dome << *m_Expr;
	dome << gen(isp, 0, -static_cast<int>(m_Expr->ReturnType()->Size()));
}

C1::AST::ReturnStmt::ReturnStmt(Expr* return_expr /*= nullptr*/)
: ExprStmt(return_expr)
{
}

void C1::AST::ReturnStmt::Dump(std::ostream& os) const
{
	os << "return " << *m_Expr << ";" << endl;
}

void C1::AST::ReturnStmt::Generate(C1::PCode::CodeDome& dome)
{
	if (m_Expr)
	{
		dome << *m_Expr;
		auto addr = ControlingFunction()->ReturnValueOffset();
		dome << gen(sto, 1, addr);
	}
	dome << gen(opr, 0, OP_RET);
	// Full the return value
}

FunctionDeclaration* C1::AST::ReturnStmt::ControlingFunction()
{
	auto node = Parent();
	FunctionDeclaration* func = dynamic_cast<FunctionDeclaration*>(node);
	while (node && !func)
	{
		node = node->Parent();
		func = dynamic_cast<FunctionDeclaration*>(node);
	}
	if (node == nullptr) return nullptr;
	else return func;
}

C1::AST::ContinueStmt::ContinueStmt()
{

}

void C1::AST::ContinueStmt::Dump(std::ostream& os) const
{
	os << "continue;" << endl;
}

C1::AST::BreakStmt::BreakStmt()
{

}

void C1::AST::BreakStmt::Dump(std::ostream& os) const
{
	os << "break;" << endl;
}

C1::AST::IterationStmt::IterationStmt(Stmt* action /*= nullptr*/, Expr* condtion /*= nullptr*/)
: m_Action(action), m_Condition(condtion)
{
	m_Action->SetParent(this);
	m_Condition->SetParent(this);
}

void C1::AST::NullStmt::Dump(std::ostream& os) const
{
	os << ";" << endl;
}

void C1::AST::NullStmt::Generate(C1::PCode::CodeDome& dome)
{
}


C1::AST::ReadStmt::ReadStmt(Expr* expr)
: ExprStmt(expr)
{
	if (expr->ValueType() != LValue)
	{
		error(this, "argument in Read statement must be L-Value");
	}
	auto rtype = expr->ReturnType();
	if (!rtype->IsBasicType())
	{
		error(this, "argument in Read statement must have basic type");
	}
}

void C1::AST::ReadStmt::Dump(std::ostream& os) const
{
	os << "read(" << *m_Expr << ");" << endl;
}

void C1::AST::ReadStmt::Generate(C1::PCode::CodeDome& dome)
{
	auto type = m_Expr->ReturnType();
	if (type->IsIntegerType()) {
		dome << gen(opr, PCode::TP_INT, OP_READ);
	}
	else
	if (type->IsFloatType()) {
		dome << gen(opr, PCode::TP_FLOAT, OP_READ);
	}
	m_Expr->GenerateLValue(dome);
	dome << gen(sar, 0, 0);
}

C1::AST::WriteStmt::WriteStmt(Expr* expr)
: ExprStmt(expr)
{
	auto type = m_Expr->ReturnType();
	type.RemoveConst();
	if (!type->IsBasicType() && type != QualType(type->AffiliatedContext()->String()))
	{
		error(this, "argument in Read statement must have basic type");
	}
	auto rtype = expr->ReturnType();
	if (rtype->IsArrayType() || rtype->IsStructType())
	{
		error(this, "can not write struct or array type");
	}
}

void C1::AST::WriteStmt::Dump(std::ostream& os) const
{
	os << "write(" << *m_Expr << ")" << endl;
}

void C1::AST::WriteStmt::Generate(C1::PCode::CodeDome& dome)
{
	dome << *m_Expr;
	auto type = m_Expr->ReturnType();
	if (type->IsIntegerType()) {
		dome << gen(opr, PCode::TP_INT, OP_WRITE);
		return;
	}
	if (type->IsFloatType()) {
		dome << gen(opr, PCode::TP_FLOAT, OP_WRITE);
		return;
	}
	type.RemoveConst();
	if (type == QualType(type->AffiliatedContext()->String())) {
		dome << gen(opr, PCode::TP_STRING, OP_WRITE);
		return;
	}
}
