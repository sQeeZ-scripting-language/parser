#ifndef EXPRESSION_NODE_HPP
#define EXPRESSION_NODE_HPP

#include <iostream>
#include <memory>

#include "lexer/tokens/token.hpp"
#include "parser/nodes/ast_node.hpp"

class ASTVisitor;

class ExpressionNode : public ASTNode {
public:
  virtual ~ExpressionNode() = default;
  virtual void accept(ASTVisitor& visitor) = 0;
  virtual std::string toString() const = 0;
};

class BinaryExpressionNode : public ExpressionNode {
public:
  std::unique_ptr<ExpressionNode> left;
  std::unique_ptr<ExpressionNode> right;
  Token op;

  BinaryExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, Token op)
      : left(std::move(left)), right(std::move(right)), op(op) {}

  void accept(ASTVisitor& visitor) override { visitor.visitBinaryExpressionNode(*this); }

  std::string toString() const override {
    std::ostringstream oss;
    oss << "###BINARY_EXPRESSION###\n###LEFT" << left->toString() << "\n###OP" << op.toString() << "\n###RIGHT" << right->toString();
    return oss.str();
  }
};

class PrimaryExpressionNode : public ExpressionNode {
public:
  Token token;

  explicit PrimaryExpressionNode(const Token& token) : token(token) {}

  void accept(ASTVisitor& visitor) override { visitor.visitPrimaryExpressionNode(*this); }

  std::string toString() const override {
    return token.toString();
  }
};

#endif
