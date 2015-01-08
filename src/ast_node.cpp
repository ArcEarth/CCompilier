#include "stdafx.h"
#include "ast_node.h"
using namespace C1::AST;


Node::Node()
{}

Node::Node(const Node::LocationType& location)
: m_location(location)
{
}

C1::AST::Node::~Node()
{

}

void C1::AST::Node::Generate(C1::PCode::CodeDome& dome)
{

}

namespace C1
{
	namespace AST{
		void error(const C1::location &loc, const std::string &msg) {
			std::cerr << "Error	: " << loc.begin.line << "." << loc.begin.column;
			std::cerr << " - " << loc.end.line << "." << loc.end.column << " ";
			std::cerr << msg << std::endl;
		}
		void error(const C1::AST::Node *node, const std::string &msg) {
			const auto & loc = node->Location();
			std::cerr << "Error	: " << loc.begin.line << "." << loc.begin.column;
			std::cerr << " - " << loc.end.line << "." << loc.end.column << " ";
			std::cerr << msg << std::endl;
			std::cerr << "->Source: " << *node << std::endl;
		}
	}
}