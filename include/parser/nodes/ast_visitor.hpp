#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "parser/nodes/ast_visitor.hpp"
#include "parser/nodes/expression_node.hpp"

class PrimaryExpressionNode;
class BinaryExpressionNode;
class AssignmentExpressionNode;
class ObjectLiteralNode;
class PropertyNode;
class MemberExpressionNode;

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void visitPrimaryExpressionNode(PrimaryExpressionNode& node) = 0;
  virtual void visitBinaryExpressionNode(BinaryExpressionNode& node) = 0;
  virtual void visitAssignmentExpressionNode(AssignmentExpressionNode& node) = 0;
  virtual void visitObjectLiteralNode(ObjectLiteralNode& node) = 0;
  virtual void visitPropertyNode(PropertyNode& node) = 0;
  virtual void visitMemberExpressionNode(MemberExpressionNode& node) = 0;
};

#endif
