#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "parser/nodes/ast_visitor.hpp"
#include "parser/nodes/expression_node.hpp"
#include "parser/nodes/function_node.hpp"
#include "parser/nodes/statement_node.hpp"

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void visitBinaryExpressionNode(BinaryExpressionNode& node) = 0;
  virtual void visitLiteralNode(LiteralNode& node) = 0;
  virtual void visitVariableNode(VariableNode& node) = 0;
  virtual void visitAssignmentNode(AssignmentNode& node) = 0;
  virtual void visitReturnNode(ReturnNode& node) = 0;
  virtual void visitFunctionNode(FunctionNode& node) = 0;
  virtual void visitProgramNode(ProgramNode& node) = 0;
};

#endif