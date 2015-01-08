#pragma once
#include "ast_node.h"
#include "expr.h"

namespace C1
{
	namespace AST
	{

		class Stmt : public Node
		{
		public:
		};

		// it's a simple ";"
		class NullStmt : public Stmt
		{
		public:
			NullStmt() {}
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		// wrap an declaration into an statement
		class DeclStmt : public Stmt
		{
		public:
			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class CompoundStmt :public Stmt, public ScopeNode, public DeclContext
		{
		public:
			CompoundStmt();
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class ExprStmt : public Stmt
		{
		public:
			explicit ExprStmt(Expr* expr);
			const Expr* Expression() const { return m_Expr.get(); }
			Expr* Expression() { return m_Expr.get(); }
			void SetExpression(Expr* expr) { m_Expr.reset(expr); }
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Expr> m_Expr;
		};

		class ReturnStmt : public ExprStmt
		{
		public:
			explicit ReturnStmt(Expr* return_expr = nullptr);
			void Dump(std::ostream&) const;

			FunctionDeclaration* ControlingFunction();
			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class ReadStmt : public ExprStmt
		{
		public:
			explicit ReadStmt(Expr*);
			void Dump(std::ostream& os) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class WriteStmt : public ExprStmt
		{
		public:
			explicit WriteStmt(Expr*);
			void Dump(std::ostream& os) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};


		class CaseLabel : public Stmt
		{
		};

		class IterationStmt : public Stmt
		{
		public:
			const Stmt* Action() const { return m_Action.get(); }
			const Expr* Condition() const { return m_Condition.get(); }
			void SetAction(Stmt* val) { m_Action.reset(val); }
			void SetCondition(Expr* val) { m_Condition.reset(val); }
			size_t EntryAddress() const { return m_EntryAddress; }
			size_t ActionEntryAddress() const { return m_ActionEntryAddress; }
			size_t ExitAddress() const { return m_ExitAddress; }
		protected:
			IterationStmt(Stmt* action = nullptr, Expr* condtion = nullptr);
			std::unique_ptr<Expr> m_Condition;
			std::unique_ptr<Stmt> m_Action;

			size_t m_EntryAddress;
			size_t m_ActionEntryAddress;
			size_t m_ExitAddress;
		};

		class ContinueStmt : public Stmt
		{
		public:
			ContinueStmt();
			void Dump(std::ostream&) const;

		protected:
			IterationStmt*	m_TargetIterationStmt;
		};

		class BreakStmt : public Stmt
		{
		public:
			BreakStmt();
			void Dump(std::ostream&) const;
		protected:
			IterationStmt*	m_TargetIterationStmt;
		};

		class WhileStmt : public IterationStmt
		{
		public:
			WhileStmt(Expr* condition, Stmt* action);
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		};

		class ForStmt : public IterationStmt
		{
		public:
			ForStmt(Stmt* initialize_stmt, Expr* condition, Expr* post_action, Stmt* action);
			const Stmt* Initializer() const { return m_Initializer.get(); }
			const ExprStmt* PostAction() const { return m_PostAction.get(); }
			void Dump(std::ostream&) const;
		protected:
			std::unique_ptr<Stmt> m_Initializer;
			std::unique_ptr<ExprStmt> m_PostAction;
		};

		class DoWhileStmt : public IterationStmt
		{
		public:
			DoWhileStmt(Stmt* action, Expr* condition);
			void Dump(std::ostream&) const;
		};

		class IfStmt : public Stmt
		{
		public:
			IfStmt(Expr* condition, Stmt* then_action, Stmt* else_action = nullptr);
			const Expr* Condition() const { return m_Condition.get(); }
			const Stmt* Then() const { return m_ThenAction.get(); }
			const Stmt* Else() const { return m_ElseAction.get(); }
			void Dump(std::ostream&) const;

			virtual void Generate(C1::PCode::CodeDome& dome);

		protected:
			std::unique_ptr<Expr> m_Condition;
			std::unique_ptr<Stmt> m_ThenAction;
			std::unique_ptr<Stmt> m_ElseAction;
		};

	}
}