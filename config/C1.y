%require "3.0"
%skeleton "lalr1.cc"
%defines 
%define api.namespace {C1}
%define api.token.constructor
%define api.value.type variant
%define parser_class_name {BisonParser}
%param { C1::AST::ASTContext& context }
%param { C1::FlexScanner& scanner }

%define parse.assert
%define parse.trace
%define parse.error verbose


%code requires {
	#include "operators.h"
	#include "type.h"
	#include "declaration.h"
	#include "ast.h"
	//#define YYDEBUG 3
	// Forward-declare the Scanner class; the Parser needs to be assigned a 
	// Scanner, but the Scanner can't be declared without the Parser
	namespace C1 {
		class FlexScanner;
		namespace AST {
			typedef AST::Expr *ExprPtr;
		}
	}
}

%code {
	// Prototype for the yylex function
	//static int yylex(C1::BisonParser::semantic_type * yylval,C1::BisonParser::location_type * yylloc,C1::AST::ASTContext&, C1::FlexScanner &scanner);
	static C1::BisonParser::symbol_type yylex(C1::AST::ASTContext&, C1::FlexScanner &scanner);
	static int debug(const char* message)
	{
		return printf(message);
	}
	using namespace C1::AST;
	using namespace std;
}

%locations
%token 
	END "eof"
	WHILE "while"
	IF "if"
	ELSE "else"
	FOR "for"
	DO "do"
	SWITCH "switch"
	GOTO "goto"
	BREAK "break"
	RETURN "return"
	CONTINUE "continue"
	CASE "case"
	DEFAULT "defaul"
	TYPEDEF "typedef"
	READ "read"
	WRITE "write"
	COMMA ","
	SEMICOLON ";"
	COLON ":"
	QUESTION "?"
	TEMPLATE "template"
	;

%token INT VOID FLOAT DOUBLE SIGNED UNSIGNED LONG CHAR SHORT BOOL

%token <StorageClassSpecifierEnum>
	STATIC "static"
	EXTERN "extern"
	AUTO "auto"
	REGISTER "register"
	;
%token <TypeQualifierEnum>
	CONST "const" 
	VOLATILE "volatile"
	RESTRICT "restrict"
	;
%token <RecordKeywordEnum>
	STRUCT "struct" 
	UNION "union"
	ENUM "enum"
	;

%token <AST::ExprPtr> INT_LITERAL FLOAT_LITERAL STRING_LITERAL BOOL_LITERAL
%token <std::string> NewIdentifier ObjectIdentifier TypeIdentifier

%token <OperatorsEnum>
	ASSIGN "="
	MUL_ASSIGN "*="
	ADD_ASSIGN "+="
	DIV_ASSIGN "/="
	SUB_ASSIGN "-="
	AND_ASSIGN "&="
	OR_ASSIGN "|="
	LSH_ASSIGN "<<="
	RSH_ASSIGN ">>="
	MOD_ASSIGN "%="
	XOR_ASSIGN "^="
	ADD "+" 
	SUB "-"
	MUL "*"
	DIV "/"
	MOD "%"
	ADDADD "++"
	SUBSUB "--"
	EQL "=="
	NEQ "!="
	LSS "<"
	GTR ">"
	LEQ "<="
	GEQ "=>"
	NOT "!"
	ANDAND "&&"
	OROR "||"
	LSH "<<"
	RSH ">>"
	REVERSE "~"
	AND "&"
	OR "|"
	XOR "^"
	DOT "."
	ARROW "->"
	SIZEOF "sizeof"
	CAST "cast"
;

%token
	LPAREN "("
	RPAREN ")"
	LBRACKET "["
	RBRACKET "]"
	LBRACE "{"
	RBRACE "}"
;

%left NOELSE
%left ELSE
%left COMMA
%right ASSIGN MUL_ASSIGN ADD_ASSIGN DIV_ASSIGN SUB_ASSIGN AND_ASSIGN OR_ASSIGN LSH_ASSIGN RSH_ASSIGN MOD_ASSIGN XOR_ASSIGN
%left OROR
%left ANDAND
%left OR
%left XOR
%left AND
%left EQL NEQ
%left LSS GTR LEQ GEQ
%left LSH RSH
%left ADD SUB
%left MUL DIV MOD
%right POSITIVE NEGTIVE NOT REVERSE ADDRESS DEREF SIZEOF ADDADD SUBSUB CAST
%left MEMBER RBRACKET RPAREN

%type <TypeQualifierEnum> TypeQualifier 
%type <int> TypeQualifierList
%type <StorageClassSpecifierEnum> StorageClassSpecifier
%type <RecordKeywordEnum> RecordKeyword 

%type <AST::Expr*> Expr AssignExpr ConstantExpr ConditionalExpr ArithmeticExpr CastExpr BitwiseExpr UnaryExpr PosfixExpr PrimaryExpr RelationExpr EqualityExpr LogicAndExpr LogicOrExpr
%type <AST::Initializer*> Initializer
%type <AST::InitializerList*> InitializerList 

%type <AST::TypeSpecifier*> TypeSpecifier EnumSpecifier
%type <AST::QualifiedTypeSpecifier*> QualifiedTypeSpecifier
%type <AST::TypeExpr*> TypeExpr
%type <AST::Stmt*> Stmt ExprStmt IterationStmt SelectionStmt JumpStmt DeclStmt Label ReadStmt WriteStmt
%type <AST::CompoundStmt*> StmtList CompoundStmt

%type <AST::TranslationUnit*> TranslationUnit 
%type <AST::Node*> ExtendDeclaration
%type <AST::FunctionDeclaration*> FunctionHeader FunctionDefination
%type <AST::StructDeclaration *> RecordHeader RecordSpecifier
%type <AST::StructBody*> StructBody FieldDeclarationList
%type <AST::VarDeclStmt*> ObjectDeclaration
%type <AST::TypedefStmt*> TypeDefination
%type <AST::ParameterDeclaration*> ParameterDeclaration
%type <AST::FieldDeclStmt*> FieldDeclaration

%type <AST::Declarator*> Declarator DirectDeclarator AbstractDeclarator DirectAbstractDeclarator InitDeclarator FieldDeclarator
%type <AST::Enumerator*> Enumerator


%type <std::string> Identifier 

%type <std::list<std::pair<OperatorsEnum,int>>*> DeclaratorPrefix
%type <std::list<AST::Declarator*>*> DeclaratorList InitDeclaratorList FieldDeclaratorList
%type <std::list<AST::Enumerator*>*> EnumeratorList
%type <AST::ParameterList*> ParameterList
%type <std::list<AST::Expr*>*> ArgumentList

%type <OperatorsEnum> UnaryOperator AssignOperator DeclaratorPrefixOperator

%type <std::list<AST::TemplateArgument>> TemplateArgumentList

%initial-action
{
// Initialize the initial location.
@$.begin.filename = @$.end.filename = &context.FileName;
};

%%
%start TranslationUnit;
TranslationUnit
	: TranslationUnit END
	{
		$$ = $1;
		$$->SetLocation(@$);
		return 0;
	}
	|
	TranslationUnit ExtendDeclaration
	{
		$$ = $1;
		$$ -> Children().push_back($2);
		$2->SetParent($$);
		$$->SetLocation(@$);
	}
	| %empty
	{
		$$ = context.CurrentTranslationUnit = new TranslationUnit(context.FileName);
		context.push_context($$);
	}
	;

ExtendDeclaration
	: FunctionDefination
	{
		$$ = $1;
	}
	| DeclStmt
	{
		$$ = $1;
	}
	;

FunctionDefination
	: FunctionHeader CompoundStmt
	{
		$$ = $1;
		$$->SetDefinition($2);
		$$->SetLocation(@$);
		context.pop_context();
	}
	;

FunctionHeader
	: StorageClassSpecifier QualifiedTypeSpecifier Declarator
	{
		auto declarator = $3;
		if (dynamic_cast<FunctionalDeclarator*>(declarator) == nullptr)
		{
			error(@3,"Expect a function declaration here.");
			// WTF, what should we do to recover here?!
			// let's wrap the declarator to declarator() and process.
			declarator = new FunctionalDeclarator(declarator,nullptr,0);
		}
		auto func = new FunctionDeclaration($1,$2,static_cast<FunctionalDeclarator*>(declarator));
		context.current_context()->add(func);
		// Add parameter list to the look up stack's top , which enabled the lookup of parameters
		context.push_context(&func->Parameters());
		$$ = func;
		$$->SetLocation(@$);

	}
	;

ObjectDeclaration
	: StorageClassSpecifier QualifiedTypeSpecifier InitDeclaratorList
	{
		//$3 will be moved.
		auto compound_decl = new VarDeclStmt($1,$2,$3);
		$$ = compound_decl;
		$$ -> SetLocation(@$);
		for (auto declarator : compound_decl->DeclaratorList())
		{
			ValueDeclaration* decl;
			if (dynamic_cast<FunctionalDeclarator*>(declarator) != nullptr)
			{
				if (context.current_context() != context.CurrentTranslationUnit)
				error(@$,"warnning : declare/define a function inside function is not allow.");
				// Recovery : assume the function is in global scope
				decl = new FunctionDeclaration($1,$2,dynamic_cast<FunctionalDeclarator*>(declarator));
				decl -> SetSourceNode(compound_decl);
				context.CurrentTranslationUnit->add(decl);
				// We don't need this, because we only push the parameters decl-context into context-stack if we entering a function body
				// Popout the parameters Decl-Context
				// context.pop_context();
			} else if (dynamic_cast<IdentifierDeclarator*>(declarator) != nullptr && dynamic_cast<FunctionType*>($2->RepresentType().get()) != nullptr)
			{
				error(@$,"warnning : WTF! declaring a function type object is not allow.");
			} else
			{
				auto var = new VariableDeclaration($1,$2->RepresentType(),declarator);
				var -> SetSourceNode(compound_decl);
				static_cast<VariableDeclaration*>(var)->ValidateInitialization();
				context.current_context()->add(var);
				compound_decl -> Declarations().push_back(var);
			}
			// add the sub-declaration into the easy access list.
		}
	}
	| StorageClassSpecifier QualifiedTypeSpecifier
	{
		//$3 will be moved.
		auto empty_list = new std::list<Declarator*>();
		auto compound_decl = new VarDeclStmt($1,$2,empty_list);
		$$ = compound_decl;
		$$ -> SetLocation(@$);
	}
	;

TypeDefination
	: TYPEDEF QualifiedTypeSpecifier DeclaratorList
	{
		$$ = new TypedefStmt($2,$3);
		for (auto declarator : $$->DeclaratorList())
		{
			auto decl = new TypedefDeclaration($2->RepresentType(),declarator);
			decl -> SetSourceNode($$);
			context.current_context()->add(decl);
			$$ -> Declarations().push_back(decl);
		}
	}
	;

DeclaratorList
	: DeclaratorList COMMA Declarator
	{
		$$ = $1;
		$$->push_back($3);
	}
	| Declarator
	{
		$$ = new std::list<Declarator*>();
		$$->push_back($1);
	}
	;

Declarator
	: DeclaratorPrefix DirectDeclarator
	{
		auto& list = *$1;
		Declarator *declarator = $2;
		for (auto itr = list.rbegin();itr!=list.rend();++itr)
		{
			declarator = new PrefixDeclarator(itr->first,itr->second,declarator);
			declarator->SetLocation(@$);
		}
		delete $1;
		$1 = nullptr;
		$$ = declarator;
		$$->SetLocation(@$);
	}
	| DirectDeclarator
	{
		$$ = $1;
	}
	;

DirectDeclarator
	: Identifier
	{
		$$ = new IdentifierDeclarator($1);
		$$->SetLocation(@$);
	}
	| LPAREN Declarator RPAREN
	{
		$$ = new ParenDeclarator($2);
		$$->SetLocation(@$);
	}
	| DirectDeclarator LBRACKET ConstantExpr RBRACKET
	{
		$$ = new ArrayDeclarator($1,$3);
		$$->SetLocation(@$);
	}
	| DirectDeclarator LBRACKET RBRACKET
	{
		$$ = new ArrayDeclarator($1,nullptr);
		$$->SetLocation(@$);
	}
	| DirectDeclarator LPAREN ParameterList RPAREN TypeQualifierList
	{
		$$ = new FunctionalDeclarator($1,$3,$5);
		$$->SetLocation(@$);
	}
	| DirectDeclarator LPAREN RPAREN TypeQualifierList
	{
		// Build an empty parameter list
		auto param_list = new ParameterList();
		$$ = new FunctionalDeclarator($1,param_list,$4);
		$$->SetLocation(@$);
	}
	;

InitDeclaratorList
	: InitDeclaratorList "," InitDeclarator
	{
		$$ = $1;
		$$->push_back($3);
	}
	| InitDeclarator
	{
		$$ = new std::list<Declarator*>();
		$$->push_back($1);
	}
	;

InitDeclarator
	: Declarator
	{
		$$ = $1;
	}
	| Declarator "=" Initializer
	{
		$$ = new InitDeclarator($1,$3);
		$$->SetLocation(@$);
	}
	;

Initializer
	: ConditionalExpr
	{
		$$ = new AtomInitializer($1);
		$$->SetLocation(@$);
	}
	| "{" InitializerList "}"
	{
		$$ = $2;
		$$->SetLocation(@$);
	}
	| "{" InitializerList COMMA "}"
	{
		$$ = $2;
		$$->SetLocation(@$);
	}
	;

InitializerList
	: InitializerList "," Initializer
	{
		$$ = $1;
		$$->push_back($3);
	}
	| Initializer
	{
		$$ = new InitializerList(context.type_context);
		$$->push_back($1);
	}
	;

AbstractDeclarator
	: DeclaratorPrefix DirectAbstractDeclarator
	{
		auto& list = *$1;
		Declarator *declarator = $2;
		for (auto itr = list.rbegin();itr!=list.rend();++itr)
		{
			declarator = new PrefixDeclarator(itr->first,itr->second,declarator);
			$$->SetLocation(@$);
		}
		delete $1;
		$1 = nullptr;
		$$ = declarator;
		$$->SetLocation(@$);
	}
	| DirectAbstractDeclarator
	{
		$$ = $1;
	}
	;

DirectAbstractDeclarator
	: LPAREN AbstractDeclarator RPAREN
	{
		$$ = new ParenDeclarator($2);
		$$->SetLocation(@$);
	}
	| LBRACKET ConstantExpr RBRACKET 
	{
		$$ = new ArrayDeclarator(nullptr,$2);
		$$->SetLocation(@$);
	}
	| LBRACKET RBRACKET
	{
		$$ = new ArrayDeclarator(nullptr,nullptr);
		$$->SetLocation(@$);
	}
	| DirectAbstractDeclarator LBRACKET ConstantExpr RBRACKET 
	{
		$$ = new ArrayDeclarator($1,$3);
		$$->SetLocation(@$);
	}
	| DirectAbstractDeclarator LBRACKET RBRACKET
	{
		$$ = new ArrayDeclarator($1,nullptr);
		$$->SetLocation(@$);
	}
	| LPAREN ParameterList RPAREN TypeQualifierList
	{
		$$ = new FunctionalDeclarator(nullptr,$2,$4);
		$$->SetLocation(@$);
	}
	| LPAREN RPAREN TypeQualifierList
	{
		$$ = new FunctionalDeclarator(nullptr,nullptr,$3);
		$$->SetLocation(@$);
	}
	| DirectAbstractDeclarator LPAREN ParameterList RPAREN TypeQualifierList
	{
		$$ = new FunctionalDeclarator($1,$3,$5);
		$$->SetLocation(@$);
		$3->SetLocation(@2 + @4);
	}
	| DirectAbstractDeclarator LPAREN RPAREN TypeQualifierList
	{
		// Build an empty parameter list
		auto param_list = new ParameterList();
		$$ = new FunctionalDeclarator($1,param_list,$4);
		$$->SetLocation(@$);
		param_list->SetLocation(@2 + @3);
	}
	;

ParameterList
	: ParameterList COMMA ParameterDeclaration
	{
		$$ = $1;
		$$->add($3);
		$3->SetParent($$);
	}
	| ParameterDeclaration
	{
		$$ = new ParameterList();
		$$->add($1);
		$1->SetParent($$);
	}
	;

ParameterDeclaration
	: QualifiedTypeSpecifier Declarator
	{
		$$ = new ParameterDeclaration($1,$2);
	}
	| QualifiedTypeSpecifier AbstractDeclarator
	{
		$$ = new ParameterDeclaration($1,$2);
	}
	| QualifiedTypeSpecifier
	{
		$$ = new ParameterDeclaration($1,new EmptyDeclarator());
	}
	;

DeclaratorPrefixOperator
	: AND
	{
		$$ = $1;
	}
	| MUL
	{
		$$ = $1;
	}
	| ANDAND
	{
		$$ = $1;
	}
	;

DeclaratorPrefix
	: DeclaratorPrefixOperator TypeQualifierList
	{
		// Pointer
		$$ = new std::list<std::pair<OperatorsEnum,int>>();
		$$->emplace_back($1,$2);
	}
	| DeclaratorPrefix DeclaratorPrefixOperator TypeQualifierList
	{
		$$->emplace_back($2,$3);
	}
	;

QualifiedTypeSpecifier
	: TypeQualifierList TypeSpecifier
	{
		$$ = new QualifiedTypeSpecifier($1,$2);
	}
	;

TypeQualifierList
	: TypeQualifierList TypeQualifier
	{
		$$ |= $1;
	}
	| %empty
	{
		$$ = 0;
	}
	;

TypeSpecifier
	: VOID
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Void());
	}
	| INT
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Int());
	}
	| FLOAT
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Float());
	}
	| CHAR
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Char());
	}
	| BOOL
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Bool());
	}
	| AUTO
	{
		$$ = new PrimaryTypeSpecifier(context.type_context->Int());
	}
	| RecordSpecifier
	{
		$$ = $1;
	}
	| EnumSpecifier
	{
		$$ = $1;
	}
	| TypeIdentifier
	{
		$$ = new TypedefNameSpecifier(context.current_context(),$1);
		$$ -> SetLocation(@$);
		/*auto decl = dynamic_cast<TypeDeclaration*>(context.current_context()->Lookup(*$1));
		assert(decl != nullptr);
		$$ = decl->DeclType();*/
	}
	;

TypeQualifier
	: CONST
	{
		$$ = $1;
	}
	| VOLATILE 
	{
		$$ = $1;
	}
	| RESTRICT
	{
		$$ = $1;
	}
	;

StorageClassSpecifier
	: EXTERN
	{
		$$ = $1;
	}
	| STATIC
	{
		$$ = $1;
	}
	| AUTO
	{
		$$ = $1;
	}
	| REGISTER
	{
		$$ = $1;
	}
	| %empty
	{
		$$ = SCS_NONE;
	}
	;

RecordSpecifier
	: RecordHeader StructBody
	{
		//current_context = *context.CurrentDeclContext;
		$$ = $1;
		$$ -> SetLocation(@$);
		dynamic_cast<StructDeclaration*>($$) -> SetDefinition($2);
	}
	| RecordHeader
	{
		$$ = $1;
	}
	| RecordKeyword StructBody
	{
		auto decl = new StructDeclaration($2);
		context.current_context()->add(decl);
		$$ = decl;
		$$ -> SetLocation(@$);
	}
	;

RecordHeader
	: RecordKeyword Identifier
	{
		auto decl = new StructDeclaration($2);
		auto res = context.current_context()->add(decl);
		if (res != DeclContext::InsertionResult::Success_CompatibleRedefinition)
		{
			auto type = context.type_context->NewRecordType($1,$2);
			decl->SetDeclType(type);
			decl->SetRepresentType(type);
		}
		$$ = decl;
		$$ -> SetLocation(@$);

		/*auto decl = current_context->LocalLookup($2);
		if (!decl)
		{
			decl = new StructDeclaration($2);
			current_context->AddDeclaration(decl);
		} else {
			auto struct_decl = dynamic_cast<StructDeclaration*>(decl);
			if (!struct_decl)
			{
				error(@2,"redefination of symbol ""%s"".");
				//Recover : replace the defination with latest one.
				decl = new StructDeclaration($2);
				current_context->AddDeclaration(decl);
			} else {
				struct_decl->AddNode(specifier);
			}
		}
		specifier->SetDecl(decl);*/
	}
	;

StructBody
	: "{" FieldDeclarationList "}"
	{
		$$ = $2;
		context.pop_context();
	}
	;

RecordKeyword
	: STRUCT
	{
		$$ = $1;
	}
	| UNION
	{
		$$ = $1;
	}
	;

EnumSpecifier
	: ENUM Identifier LBRACE EnumeratorList RBRACE
	{
		$$ = nullptr;
		//$$ = new EnumSpecifier($2,$4);
	}
	| ENUM Identifier LBRACE EnumeratorList COMMA RBRACE
	{
		$$ = nullptr;
		//$$ = new EnumSpecifier($2,$4);
	}
	| ENUM LBRACE EnumeratorList RBRACE
	{
		$$ = nullptr;
		//$$ = new EnumSpecifier($3);
	}
	| ENUM LBRACE EnumeratorList COMMA RBRACE
	{
		$$ = nullptr;
		//$$ = new EnumSpecifier($3);
	}
	| ENUM Identifier
	{
		$$ = nullptr;
		//$$ = new EnumSpecifier($2);
	}
	;

EnumeratorList
	: EnumeratorList COMMA Enumerator
	{
		$$ = $1;
		$$->push_back($3);
	}
	| Enumerator
	{
		$$ = new std::list<Enumerator*>();
		$$->push_back($1);
	}
	;

Enumerator
	: Identifier
	{
		$$ = new Enumerator($1);
		$$->SetLocation(@$);
	}
	| Identifier ASSIGN ConstantExpr
	{
		$$ = new Enumerator($1,$3);
		$$->SetLocation(@$);
	}
	;

FieldDeclarationList
	: %empty
	{
		$$ = new StructBody();
		context.push_context($$);
	}
	| FieldDeclarationList FieldDeclaration
	{
		$$ = $1;
		$$->Children().push_back($2);
		$2->SetParent($$);
	}
	;

FieldDeclaration
	: QualifiedTypeSpecifier FieldDeclaratorList SEMICOLON
	{
		auto compound_decl = new FieldDeclStmt($1,$2);
		compound_decl -> SetLocation(@$);
		for (auto declarator : compound_decl->DeclaratorList())
		{
			auto decl = new FieldDeclaration($1->RepresentType(),declarator);
			decl -> SetSourceNode(compound_decl);
			context.current_context()->add(decl);
			// Handel redefination
		}
		$$ = compound_decl;
	}
	| QualifiedTypeSpecifier SEMICOLON
	{
		auto empty_list = new std::list<Declarator*>;
		auto compound_decl = new FieldDeclStmt($1,empty_list);
		compound_decl -> SetLocation(@$);
		$$ = compound_decl;
	}
	;

FieldDeclaratorList
	: FieldDeclaratorList COMMA FieldDeclarator
	{
		$$ = $1;
		$$->push_back($3);
	}
	| FieldDeclarator
	{
		$$ = new std::list<Declarator*>();
		$$->push_back($1);
	}
	;

FieldDeclarator
	: Declarator COLON ConstantExpr
	{
		$$ = new FieldDeclarator($1,$3);
	}
	| COLON ConstantExpr
	{
		$$ = new FieldDeclarator(nullptr,$2);
	}
	| Declarator
	{
		$$ = new FieldDeclarator($1);
	}
	;

Identifier
	: NewIdentifier
	{
		$$ = $1;
	}
	| ObjectIdentifier
	{
		$$ = $1;
	}
	;

PrimaryExpr
	: INT_LITERAL
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	| STRING_LITERAL
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	| FLOAT_LITERAL
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	| BOOL_LITERAL
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	| Identifier
	{
		auto refExpr = DeclRefExpr::MakeDeclRefExpr(context.current_context(),context.type_context,$1);
		refExpr->SetLocation(@$);
		// Resolve reference after setup location
		refExpr->ResoloveReference();
		$$ = refExpr;
	}
	| LPAREN Expr RPAREN
	{
		$$ = new ParenExpr($2);
		$$->SetLocation(@$);
	}
	;

PosfixExpr
	: PrimaryExpr
	{
		$$ = $1;
	}
	| PosfixExpr ADDADD
	{
		$$ = PosfixExpr::MakePosfixExpr(OP_POSFIX_ADDADD,$1);
		$$->SetLocation(@$);
	}
	| PosfixExpr SUBSUB
	{
		$$ = PosfixExpr::MakePosfixExpr(OP_POSFIX_SUBSUB,$1);
		$$->SetLocation(@$);
	}
	| PosfixExpr "[" Expr "]"
	{
		$$ = IndexExpr::MakeIndexExpr($1,$3);
		$$->SetLocation(@$);
	}
	| PosfixExpr LPAREN ArgumentList RPAREN
	{
		$$ = CallExpr::MakeCallExpr($1,$3);
		$$->SetLocation(@$);
	}
	| PosfixExpr LPAREN RPAREN
	{
		auto empty_list = new std::list<AST::Expr*>();
		$$ = CallExpr::MakeCallExpr($1,empty_list);
		$$->SetLocation(@$);
	}
	| PosfixExpr DOT Identifier
	{
		$$ = MemberExpr::MakeMemberExpr($1,$3,OP_DOT);
		$$->SetLocation(@$);
	}
	| PosfixExpr ARROW Identifier
	{
		$$ = MemberExpr::MakeMemberExpr($1,$3,OP_ARROW);
		$$->SetLocation(@$);
	}
	;

UnaryExpr
	: PosfixExpr
	{
		$$ = $1;
	}
	| "*" CastExpr
	{
		$$ = DereferenceExpr::MakeDereferenceExpr($2);
		$$->SetLocation(@$);
	}
	| "&" CastExpr %prec ADDRESS
	{
		$$ = AddressOfExpr::MakeAddressOfExpr($2);
		$$->SetLocation(@$);
	}
	| ADDADD UnaryExpr
	{
		$$ = UnaryExpr::MakeUnaryExpr($1,$2);
		$$->SetLocation(@$);
	}
	| SUBSUB UnaryExpr
	{
		$$ = UnaryExpr::MakeUnaryExpr($1,$2);
		$$->SetLocation(@$);
	}
	| UnaryOperator CastExpr
	{
		$$ = UnaryExpr::MakeUnaryExpr($1,$2);
		$$->SetLocation(@$);
	}
	| SIZEOF UnaryExpr
	{
		$$ = new SizeofExpr($2);
		$$->SetLocation(@$);
	}
	| SIZEOF LPAREN TypeExpr RPAREN
	{
		$$ = new SizeofTypeExpr($3);
		$$->SetLocation(@$);
	}
	;

UnaryOperator
	: ADD
	{
		$$ = $1;
	}
	| SUB
	{
		$$ = OP_NEG;
	}
	| REVERSE
	{
		$$ = $1;
	}
	| NOT
	{
		$$ = $1;
	}
	;

CastExpr
	: UnaryExpr
	{
		$$ = $1;
	}
	| CAST LSS TypeExpr GTR LPAREN Expr RPAREN
	{
		$$ = new ExplicitCastExpr($3,$6);
		$$->SetLocation(@$);
	}
	| LPAREN TypeExpr RPAREN CastExpr
	{
		$$ = new ExplicitCastExpr($2,$4);
		$$->SetLocation(@$);
	}
	;

ArithmeticExpr
	: CastExpr
	{ 
		$$ = $1;
	}
	| ArithmeticExpr ADD CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr SUB CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr MUL CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr DIV CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr MOD CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr LSH CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| ArithmeticExpr RSH CastExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;

RelationExpr
	: ArithmeticExpr
	{
		$$ = $1;
	}
	| RelationExpr LEQ ArithmeticExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| RelationExpr GEQ ArithmeticExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| RelationExpr GTR ArithmeticExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| RelationExpr LSS ArithmeticExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;

EqualityExpr
	: RelationExpr
	{
		$$ = $1;
	}
	| EqualityExpr "==" RelationExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| EqualityExpr "!=" RelationExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;


BitwiseExpr
	: EqualityExpr
	{
		$$ = $1;
	}
	| BitwiseExpr AND EqualityExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| BitwiseExpr OR  EqualityExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	| BitwiseExpr XOR EqualityExpr
	{
		$$ = ArithmeticExpr::MakeArithmeticExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;

LogicAndExpr
	: BitwiseExpr
	{
		$$ = $1;
	}
	| LogicAndExpr "&&" BitwiseExpr
	{
		$$ = LogicExpr::MakeLogicExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;

LogicOrExpr
	: LogicAndExpr
	{
		$$ = $1;
	}
	| LogicOrExpr "||" LogicAndExpr
	{
		$$ = LogicExpr::MakeLogicExpr($2,$1,$3);
		$$->SetLocation(@$);
	}
	;

ConditionalExpr
	: LogicOrExpr
	{
		$$ = $1;
	};

AssignExpr
	: ConditionalExpr
	{
		$$ = $1;
	}
	| UnaryExpr AssignOperator AssignExpr
	{
		//Unary op_enum here is because binary op_enum don't have L-value
		$$ = AssignExpr::MakeAssignExpr($2,$1,$3);
		$$ -> SetLocation(@$);
		if ($$ -> ValueType() != LValue)
		{
			error (@1,"Semantic error : Assigned expression must have a L-Value.");
		}
	}
	;

AssignOperator
	: ASSIGN
	{ $$ = $1; }
	| MUL_ASSIGN
	{ $$ = $1; }
	| DIV_ASSIGN
	{ $$ = $1; }
	| MOD_ASSIGN
	{ $$ = $1; }
	| ADD_ASSIGN
	{ $$ = $1; }
	| SUB_ASSIGN
	{ $$ = $1; }
	| LSH_ASSIGN
	{ $$ = $1; }
	| RSH_ASSIGN
	{ $$ = $1; }
	| AND_ASSIGN
	{ $$ = $1; }
	| XOR_ASSIGN
	{ $$ = $1; }
	| OR_ASSIGN
	{ $$ = $1; }
	;

Expr
	: AssignExpr
	{
		$$ = $1;
	}
	| Expr COMMA AssignExpr
	{
		// Comma expression is evil!!!
		$$ = new CommaExpr($1,$3);
	}
	;

ConstantExpr
	: ConditionalExpr
	{
		// Constant expression don't support assign expression
		// nessary for initializers and compile time constant
		$$ = $1;
		if (!$$ -> Evaluatable())
		{
			error (@$,"Semantic error : can not evalaute the expression at compile time.");
		}
	}

ArgumentList
	: ArgumentList "," AssignExpr
	{
		//cout << "ArgList : ArgList , AssignExpr" << endl;
		$$ = $1;
		$$->push_back($3);
	}
	| AssignExpr
	{
		//cout << "ArgList : AssignExpr" << endl;
		$$ = new std::list<Expr*>();
		$$->push_back($1);
	}
	;

TypeExpr
	: QualifiedTypeSpecifier AbstractDeclarator
	{
		$$ = new TypeExpr($1,$2);
		$$->SetLocation(@$);
	}
	| QualifiedTypeSpecifier
	{
		$$ = new TypeExpr($1,nullptr);
		$$->SetLocation(@$);
	}
	;

Stmt
	: ExprStmt
	{
		$$ = $1;
	}
	| DeclStmt
	{
		$$ = $1;
	}
	| IterationStmt
	{
		$$ = $1;
	}
	| SelectionStmt
	{
		$$ = $1;
	}
	| JumpStmt
	{
		$$ = $1;
	}
	| ReadStmt
	{
		$$ = $1;
	}
	| WriteStmt
	{
		$$ = $1;
	}
	| Label
	{
		$$ = $1;
	}
	| CompoundStmt
	{
		$$ = $1;
	}
	;

CompoundStmt
	: "{" StmtList "}"
	{
		$$ = $2;
		$$->SetLocation(@$);
		context.pop_context();
	}
	;

StmtList
	: StmtList Stmt
	{
		$$ = $1;
		$$->Children().push_back($2);
		$2->SetParent($$);
	}
	| %empty
	{
		$$ = new CompoundStmt();
		context.push_context($$);
	}
	;

ReadStmt
	: "read" "(" Expr ")" ";"
	{
		$$ = new ReadStmt($3);
		$$->SetLocation(@$);
	}
	;
WriteStmt
	: "write" "(" Expr ")" ";"
	{
		$$ = new WriteStmt($3);
		$$->SetLocation(@$);
	}
	;

ExprStmt
	: Expr SEMICOLON
	{
		$$ = new ExprStmt($1);
		$$->SetLocation(@$);
	}
	| SEMICOLON
	{
		$$ = new NullStmt();
		$$->SetLocation(@$);
	}
	;

DeclStmt
	: ObjectDeclaration SEMICOLON
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	| TypeDefination SEMICOLON
	{
		$$ = $1;
		$$->SetLocation(@$);
	}
	;

IterationStmt
	: FOR "(" Stmt ExprStmt Expr ")" Stmt
	{
		//Stmt* post_action = new ExprStmt($5);
		ExprStmt* expr_stmt = dynamic_cast<ExprStmt*>($4);
		Expr* condition;
		if (expr_stmt != nullptr)
		{
			condition = expr_stmt->Expression();

			// delete this expr-stmt since we are only interested in the expr it self
			expr_stmt->SetExpression(nullptr);
			delete expr_stmt;
		} else // it's a null-statement
		{
			condition = nullptr;
		}

		$$ = new ForStmt($3,condition,$5,$7);
		$$->SetLocation(@$);
	}
	| FOR "(" Stmt ExprStmt ")" Stmt
	{
		ExprStmt* expr_stmt = dynamic_cast<ExprStmt*>($4);
		Expr* condition;
		if (expr_stmt != nullptr)
		{
			condition = expr_stmt->Expression();

			// delete this expr-stmt since we are only interested in the expr it self
			expr_stmt->SetExpression(nullptr);
			delete expr_stmt;
		} else // it's a null-statement
		{
			condition = nullptr;
		}
		$$ = new ForStmt($3,condition,nullptr,$6);
		$$->SetLocation(@$);
	}
	| WHILE "(" Expr ")" Stmt
	{
		$$ = new WhileStmt($3,$5);
		$$->SetLocation(@$);
	}
	| DO Stmt WHILE "(" Expr ")" ";"
	{
		$$ = new DoWhileStmt($2,$5);
		$$->SetLocation(@$);
	}
	;

SelectionStmt
	: "if" "(" Expr ")" Stmt %prec NOELSE
	{
		$$ = new IfStmt($3,$5);
		$$->SetLocation(@$);
	}
	| "if" "(" Expr ")" Stmt "else" Stmt
	{
		$$ = new IfStmt($3,$5,$7);
		$$->SetLocation(@$);
	}
	| "switch" "(" Expr ")" Stmt
	{
		//$$ = new SwitchStmt($3);
		//$$->SetLocation(@$);
		$$ = new NullStmt();
		//diag_context->NewDiagMsg(@$,UnsupportedFeature,"'switch statement'");
	}
	;

JumpStmt
	: CONTINUE SEMICOLON
	{
		$$ = new ContinueStmt();
		$$->SetLocation(@$);
	}
	| BREAK SEMICOLON
	{
		$$ = new BreakStmt();
		$$->SetLocation(@$);
	}
	| RETURN SEMICOLON
	{
		$$ = new ReturnStmt();
		$$->SetLocation(@$);
	}
	| RETURN Expr SEMICOLON
	{
		$$ = new ReturnStmt($2);
		$$->SetLocation(@$);
	}
	;

Label
	: CASE Expr COLON
	{
		$$ = new NullStmt();
		$$->SetLocation(@$);
	}
	| DEFAULT COLON
	{
		$$ = new NullStmt();
		$$->SetLocation(@$);
	}
	;

%%

// We have to implement the error function
void C1::BisonParser::error(const C1::BisonParser::location_type &loc, const std::string &msg) {
	std::cerr << "Error " << loc.begin.line << "." << loc.begin.column ;
	std::cerr << " - "  << loc.end.line << "." << loc.end.column << " " ;
	std::cerr << msg << std::endl;
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function
#include "cscanner.h"
inline C1::BisonParser::symbol_type yylex(C1::AST::ASTContext&, C1::FlexScanner &scanner)
{
	return scanner.NextToken();
}

/*
int yyerror(char *message)
{
	printf("%s\n",message);
	return 0;
}

ASTree parse()
{
	ASTree tree = newAST();
	ast = tree;
	yyparse();
	return tree;
} */