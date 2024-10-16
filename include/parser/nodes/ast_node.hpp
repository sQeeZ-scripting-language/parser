#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include "parser/nodes/ast_visitor.hpp"

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor& visitor) = 0;
  virtual std::string toString() const = 0;
};

#endif