#include "stdafx.h"
#include "translation_unit.h"
#include "declaration.h"
#include "decl_stmt.h"

using namespace C1;
using namespace C1::AST;
using namespace std;
using namespace C1::PCode;

void C1::AST::TranslationUnit::Dump(std::ostream& ostr) const
{
	for (auto node : Children())
	{
		ostr << * node;
	}
}

C1::AST::TranslationUnit::TranslationUnit(std::string filename)
:m_FileName(filename)
{
}

C1::AST::TranslationUnit::~TranslationUnit()
{
	set_parent(nullptr);
}

void C1::AST::TranslationUnit::Generate(C1::PCode::CodeDome& dome)
{
	dome.SP = PCode::CodeDome::CallStorageSize;
	//size_t local_var_size = 0;
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
				//local_var_size += varsize;
				dome.SP += varsize;
			}
		}
	}

	for (auto node : Children())
	{
		auto value = dynamic_cast<VarDeclStmt*>(node);
		if (value)
			dome << *value;
	}
	int cx = dome.EmplaceInstruction(cal, 0, 0);
	dome << gen(opr, 0, OP_RET);
	for (auto node : Children())
	{
		auto func = dynamic_cast<FunctionDeclaration*>(node);
		if (func && func->Definition())
			dome << *func;
	}
	auto main = dynamic_cast<FunctionDeclaration*>(this->lookup("main"));
	if (!main)
		error(this, "can not find entry point of application (main function).");
	else
		dome.Instruction(cx).a = main->Offset();
}
