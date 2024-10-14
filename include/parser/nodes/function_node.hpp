#ifndef FUNCTION_NODE_HPP
#define FUNCTION_NODE_HPP

#include <iostream>
#include <vector>

#include "parser/nodes/ast_node.hpp"
#include "parser/nodes/statement_node.hpp"

class FunctionNode : public ASTNode {
public:
  std::string name;
  std::vector<std::string> parameters;
  std::vector<std::unique_ptr<StatementNode>> body;

  FunctionNode(const std::string& name, std::vector<std::string> parameters,
               std::vector<std::unique_ptr<StatementNode>> body)
      : name(name), parameters(std::move(parameters)), body(std::move(body)) {}

  void accept(ASTVisitor& visitor) override { visitor.visitFunctionNode(*this); }
};

class ProgramNode : public ASTNode {
public:
  std::vector<std::unique_ptr<FunctionNode>> functions;

  explicit ProgramNode(std::vector<std::unique_ptr<FunctionNode>> functions) : functions(std::move(functions)) {}

  void accept(ASTVisitor& visitor) override { visitor.visitProgramNode(*this); }
};

#endif