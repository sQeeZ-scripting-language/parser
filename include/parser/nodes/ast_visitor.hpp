#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "parser/nodes/ast_visitor.hpp"
#include "parser/nodes/expression_node.hpp"

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void visitBinaryExpressionNode(BinaryExpressionNode& node) = 0;
  virtual void visitPrimaryExpressionNode(PrimaryExpressionNode& node) = 0;
};

#endif