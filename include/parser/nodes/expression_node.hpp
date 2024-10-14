#ifndef EXPRESSION_NODE_HPP
#define EXPRESSION_NODE_HPP

#include <iostream>

#include "lexer/tokens/token.hpp"
#include "parser/nodes/ast_node.hpp"

class ExpressionNode : public ASTNode {
public:
  virtual ~ExpressionNode() = default;
};

class BinaryExpressionNode : public ExpressionNode {
public:
  std::unique_ptr<ExpressionNode> left;
  std::unique_ptr<ExpressionNode> right;
  Token op;

  BinaryExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, Token op)
      : left(std::move(left)), right(std::move(right)), op(op) {}

  void accept(ASTVisitor& visitor) override { visitor.visitBinaryExpressionNode(*this); }
};

class LiteralNode : public ExpressionNode {
public:
  std::variant<int, double, std::string> value;

  explicit LiteralNode(const std::variant<int, double, std::string>& value) : value(value) {}

  void accept(ASTVisitor& visitor) override { visitor.visitLiteralNode(*this); }
};

class VariableNode : public ExpressionNode {
public:
  std::string name;

  explicit VariableNode(const std::string& name) : name(name) {}

  void accept(ASTVisitor& visitor) override { visitor.visitVariableNode(*this); }
};

#endif