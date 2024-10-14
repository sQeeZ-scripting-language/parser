#ifndef STATEMENT_NODE_HPP
#define STATEMENT_NODE_HPP

#include "parser/nodes/ast_node.hpp"

class StatementNode : public ASTNode {
public:
  virtual ~StatementNode() = default;
};

class AssignmentNode : public StatementNode {
public:
  std::string variable;
  std::unique_ptr<ExpressionNode> expression;

  AssignmentNode(const std::string& variable, std::unique_ptr<ExpressionNode> expression)
      : variable(variable), expression(std::move(expression)) {}

  void accept(ASTVisitor& visitor) override { visitor.visitAssignmentNode(*this); }
};

class ReturnNode : public StatementNode {
public:
  std::unique_ptr<ExpressionNode> expression;

  explicit ReturnNode(std::unique_ptr<ExpressionNode> expression) : expression(std::move(expression)) {}

  void accept(ASTVisitor& visitor) override { visitor.visitReturnNode(*this); }
};

#endif