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

class PrimaryExpressionNode : public ExpressionNode {
public:
  Token token;

  explicit PrimaryExpressionNode(const Token& token) : token(token) {}

  void accept(ASTVisitor& visitor) override { visitor.visitPrimaryExpressionNode(*this); }

  std::string toString() const override {
    return token.toString();
  }

  const Token& getToken() const {
    return token;
  }
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

class AssignmentExpressionNode : public ExpressionNode {
public:
  std::unique_ptr<ExpressionNode> left;
  std::unique_ptr<ExpressionNode> value;

  AssignmentExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> value)
      : left(std::move(left)), value(std::move(value)) {}

  void accept(ASTVisitor& visitor) override { 
    visitor.visitAssignmentExpressionNode(*this); 
  }

  std::string toString() const override {
    std::ostringstream oss;
    oss << "###ASSIGNMENT_EXPRESSION###\n###ASSIGNEE### " << left->toString() << "\n###VALUE### " << value->toString();
    return oss.str();
  }
};

class ObjectLiteralNode : public ExpressionNode {
public:
  std::vector<std::unique_ptr<PropertyNode>> properties;

  explicit ObjectLiteralNode(std::vector<std::unique_ptr<PropertyNode>> properties)
      : properties(std::move(properties)) {}

  void accept(ASTVisitor& visitor) override { 
    visitor.visitObjectLiteralNode(*this); 
  }

  std::string toString() const override {
    std::ostringstream oss;
    oss << "###OBJECT_LITERAL_NODE###\n###PROPERTIES###\n";
    for (const auto& property : properties) {
      oss << property->toString() << "\n";
    }
    return oss.str();
  }
};

class PropertyNode : public ExpressionNode {
public:
  std::unique_ptr<ExpressionNode> object;
  Token property;

  PropertyNode(std::unique_ptr<ExpressionNode> object, Token property)
      : object(std::move(object)), property(property) {}

  void accept(ASTVisitor& visitor) override { visitor.visitPropertyNode(*this); }

  std::string toString() const override {
    std::ostringstream oss;
    oss << "###PROPERTY_NODE###\n###OBJECT" << object->toString() 
        << "\n###PROPERTY" << property.toString();
    return oss.str();
  }
};

class MemberExpressionNode : public ExpressionNode {
public:
  std::unique_ptr<ExpressionNode> object;
  std::unique_ptr<ExpressionNode> property;
  bool computed;

  MemberExpressionNode(std::unique_ptr<ExpressionNode> object, std::unique_ptr<ExpressionNode> property, bool computed)
      : object(std::move(object)), property(std::move(property)), computed(computed) {}

  void accept(ASTVisitor& visitor) override { 
    visitor.visitMemberExpressionNode(*this); 
  }

  std::string toString() const override {
    std::ostringstream oss;
    oss << "###MEMBER_EXPRESSION###\n###OBJECT### " << object->toString() 
        << "\n###PROPERTY### " << property->toString()
        << "\n###COMPUTED### " << (computed ? "true" : "false");
    return oss.str();
  }
};



#endif
