#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/tokens/token.hpp"

enum class NodeType {
  // STATEMENTS
  Program,
  FunctionDeclaration,
  ReturnStmt,
  VarDeclaration,
  ConditionalStatement,
  WhileStatement,
  DoWhileStatement,
  ForStatement,
  ForInStatement,
  ForOfStatement,
  LogStmt,
  // EXPRESSIONS
  AssignmentExpr,
  CompoundAssignmentExpr,
  TernaryExpr,
  BinaryExpr,
  UnaryExpr,
  CallExpr,
  MemberExpr,
  // LITERALS
  Property,
  ObjectLiteral,
  ArrayLiteral,
  Identifier,
  NullLiteral,
  IntegerLiteral,
  DoubleLiteral,
  BooleanLiteral,
  CharLiteral,
  StringLiteral,
  HexCodeLiteral,
  // Short Notation
  ShortOperationLiteral,
  ShortSingleExpressionLiteral,
  ShortDoubleExpressionLiteral
};

// Base class for all AST nodes
class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual std::string toString() const = 0;
};

// Statements (do not result in a runtime value)
class Stmt : public ASTNode {
public:
  NodeType kind;
  explicit Stmt(const NodeType& kind) : kind(kind) {}
};

// Expressions (result in a runtime value)
class Expr : public Stmt {
public:
  explicit Expr(const NodeType& kind) : Stmt(kind) {}
};

// Statements
class Program : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> body;

  Program() : Stmt(NodeType::Program) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "Program:\n";
    for (const auto& stmt : body) {
      if (stmt) {
        oss << "  " << stmt->toString() << "\n";
      }
    }
    return oss.str();
  }
};

class FunctionDeclaration : public Stmt {
public:
  Token name;
  std::vector<Token> parameters;
  std::vector<std::unique_ptr<Stmt>> body;

  FunctionDeclaration(const Token& name, std::vector<Token> parameters, std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::FunctionDeclaration), name(name), parameters(std::move(parameters)), body(std::move(body)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "FunctionDeclaration: " << name.value << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
      oss << parameters[i].value;
      if (i < parameters.size() - 1) {
        oss << ", ";
      }
    }
    oss << ")\n";
    for (const auto& stmt : body) {
      oss << "  " << stmt->toString() << "\n";
    }
    return oss.str();
  }
};

class ReturnStmt : public Stmt {
public:
  std::unique_ptr<Expr> value;

  explicit ReturnStmt(std::unique_ptr<Expr> value) : Stmt(NodeType::ReturnStmt), value(std::move(value)) {}

  std::string toString() const override { return "ReturnStmt: " + (value ? value->toString() : "null"); }
};

class VarDeclaration : public Stmt {
public:
  Token type;
  Token identifier;
  std::unique_ptr<ASTNode> value;

  VarDeclaration(const Token& type, const Token& identifier, std::unique_ptr<ASTNode> value)
      : Stmt(NodeType::VarDeclaration), type(type), identifier(identifier), value(std::move(value)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "VarDeclaration: " << type.value << " " << identifier.value;
    if (value) {
      oss << " = " << value->toString();
    }
    return oss.str();
  }
};

class ConditionalStatement : public Stmt {
public:
  std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>> ifClause;
  std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>>> elifClauses;
  std::vector<std::unique_ptr<Stmt>> elseBody;

  ConditionalStatement(std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>> ifClause,
                       std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>>> elifClauses,
                       std::vector<std::unique_ptr<Stmt>> elseBody)
      : Stmt(NodeType::ConditionalStatement),
        ifClause(std::move(ifClause)),
        elifClauses(std::move(elifClauses)),
        elseBody(std::move(elseBody)) {}

  std::string toString() const override {
    std::string result = "ConditionalStatement: ";
    result += "if (" + ifClause.first->toString() + ") {\n";
    for (const auto& stmt : ifClause.second) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  }\n";
    for (const auto& elifClause : elifClauses) {
      result += "  elif (" + elifClause.first->toString() + ") {\n";
      for (const auto& stmt : elifClause.second) {
        result += "    " + stmt->toString() + "\n";
      }
      result += "  }\n";
    }
    if (!elseBody.empty()) {
      result += "  else {\n";
      for (const auto& stmt : elseBody) {
        result += "    " + stmt->toString() + "\n";
      }
      result += "  }";
    }
    return result;
  }
};

class WhileStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::vector<std::unique_ptr<Stmt>> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::WhileStatement), condition(std::move(condition)), body(std::move(body)) {}

  std::string toString() const override {
    std::string result = "WhileStatement: ";
    result += "while (" + condition->toString() + ") {\n";
    for (const auto& stmt : body) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  }";
    return result;
  }
};

class DoWhileStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::vector<std::unique_ptr<Stmt>> body;

  DoWhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::DoWhileStatement), condition(std::move(condition)), body(std::move(body)) {}

  std::string toString() const override {
    std::string result = "DoWhileStmt: ";
    result += "do {\n";
    for (const auto& stmt : body) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  } while (" + condition->toString() + ")";
    return result;
  }
};

class ForStmt : public Stmt {
public:
  std::unique_ptr<Stmt> iterator;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> increment;
  std::vector<std::unique_ptr<Stmt>> body;

  ForStmt(std::unique_ptr<Stmt> iterator, std::unique_ptr<Expr> condition, std::unique_ptr<Expr> increment,
          std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::ForStatement),
        iterator(std::move(iterator)),
        condition(std::move(condition)),
        increment(std::move(increment)),
        body(std::move(body)) {}

  std::string toString() const override {
    std::string result = "ForStmt: ";
    result += "for (";
    if (iterator) {
      result += iterator->toString();
    }
    result += "; ";
    if (condition) {
      result += condition->toString();
    }
    result += "; ";
    if (increment) {
      result += increment->toString();
    }
    result += ") {\n";
    for (const auto& stmt : body) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  }";
    return result;
  }
};

class ForInStmt : public Stmt {
public:
  std::unique_ptr<Stmt> iterator;
  std::unique_ptr<Expr> iterable;
  std::vector<std::unique_ptr<Stmt>> body;

  ForInStmt(std::unique_ptr<Stmt> iterator, std::unique_ptr<Expr> iterable, std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::ForInStatement),
        iterator(std::move(iterator)),
        iterable(std::move(iterable)),
        body(std::move(body)) {}

  std::string toString() const override {
    std::string result = "ForInStmt: ";
    result += "for (" + iterator->toString() + " in " + iterable->toString() + ") {\n";
    for (const auto& stmt : body) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  }";
    return result;
  }
};

class ForOfStmt : public Stmt {
public:
  std::unique_ptr<Stmt> iterator;
  std::unique_ptr<Expr> iterable;
  std::vector<std::unique_ptr<Stmt>> body;

  ForOfStmt(std::unique_ptr<Stmt> iterator, std::unique_ptr<Expr> iterable, std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::ForOfStatement),
        iterator(std::move(iterator)),
        iterable(std::move(iterable)),
        body(std::move(body)) {}

  std::string toString() const override {
    std::string result = "ForOfStmt: ";
    result += "for (" + iterator->toString() + " of " + iterable->toString() + ") {\n";
    for (const auto& stmt : body) {
      result += "    " + stmt->toString() + "\n";
    }
    result += "  }";
    return result;
  }
};

class LogStmt : public Stmt {
public:
  Token logType;
  std::unique_ptr<Expr> message;
  std::unique_ptr<Expr> color;

  LogStmt(const Token& logType, std::unique_ptr<Expr> message, std::unique_ptr<Expr> color = nullptr)
      : Stmt(NodeType::LogStmt), logType(logType), message(std::move(message)), color(std::move(color)) {}

  virtual std::string toString() const override {
    std::stringstream ss;
    ss << logType.plainText << "(" << message->toString();
    if (color) {
      ss << ", " << color->toString();
    }
    ss << ")";
    return ss.str();
  }
};

// Expressions
class AssignmentExpr : public Expr {
public:
  std::unique_ptr<Expr> assignee;
  std::unique_ptr<Expr> value;

  AssignmentExpr(std::unique_ptr<Expr> assignee, std::unique_ptr<Expr> value)
      : Expr(NodeType::AssignmentExpr), assignee(std::move(assignee)), value(std::move(value)) {}

  std::string toString() const override {
    return "AssignmentExpr: " + assignee->toString() + " = " + value->toString();
  }
};

class CompoundAssignmentExpr : public Expr {
public:
  std::unique_ptr<Expr> assignee;
  std::unique_ptr<Expr> value;
  Token operator_;

  CompoundAssignmentExpr(std::unique_ptr<Expr> assignee, std::unique_ptr<Expr> value, const Token& operator_)
      : Expr(NodeType::CompoundAssignmentExpr),
        assignee(std::move(assignee)),
        value(std::move(value)),
        operator_(operator_) {}

  std::string toString() const override {
    return "CompoundAssignmentExpr: " + assignee->toString() + " " + operator_.value + " " + value->toString();
  }
};

class TernaryExpr : public Expr {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> trueExpr;
  std::unique_ptr<Expr> falseExpr;

  TernaryExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> trueExpr, std::unique_ptr<Expr> falseExpr)
      : Expr(NodeType::TernaryExpr),
        condition(std::move(condition)),
        trueExpr(std::move(trueExpr)),
        falseExpr(std::move(falseExpr)) {}

  std::string toString() const override {
    return "TernaryExpr: " + condition->toString() + " ? " + trueExpr->toString() + " : " + falseExpr->toString();
  }
};

class BinaryExpr : public Expr {
public:
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  Token operator_;

  BinaryExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, const Token& operator_)
      : Expr(NodeType::BinaryExpr), left(std::move(left)), right(std::move(right)), operator_(operator_) {}

  std::string toString() const override {
    return "BinaryExpr: (" + left->toString() + " " + operator_.value + " " + right->toString() + ")";
  }
};

class UnaryExpr : public Expr {
public:
  Token operator_;
  std::unique_ptr<Expr> operand;
  bool isPrefix;

  UnaryExpr(Token operator_, std::unique_ptr<Expr> operand, bool isPrefix)
      : Expr(NodeType::UnaryExpr), operator_(operator_), operand(std::move(operand)), isPrefix(isPrefix) {}

  std::string toString() const override {
    if (isPrefix) {
      return operator_.value + operand->toString();
    } else {
      return operand->toString() + operator_.value;
    }
  }
};

class CallExpr : public Expr {
public:
  std::unique_ptr<Expr> caller;
  std::vector<std::unique_ptr<Expr>> args;

  CallExpr() : Expr(NodeType::CallExpr), caller(nullptr), args() {}

  CallExpr(std::unique_ptr<Expr> caller, std::vector<std::unique_ptr<Expr>> args)
      : Expr(NodeType::CallExpr), caller(std::move(caller)), args(std::move(args)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "CallExpr: " << caller->toString() << "(";
    for (size_t i = 0; i < args.size(); ++i) {
      oss << args[i]->toString();
      if (i < args.size() - 1) {
        oss << ", ";
      }
    }
    oss << ")";
    return oss.str();
  }
};

class MemberExpr : public Expr {
public:
  std::unique_ptr<Expr> object;
  std::unique_ptr<Expr> property;
  bool computed;

  MemberExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> property, bool computed)
      : Expr(NodeType::MemberExpr), object(std::move(object)), property(std::move(property)), computed(computed) {}

  std::string toString() const override {
    return "MemberExpr: " + object->toString() +
           (computed ? "[" + property->toString() + "]" : "." + property->toString());
  }
};

// Literals / Primary Expressions
class Property : public Expr {
public:
  Token key;
  std::unique_ptr<Expr> value;

  Property(const Token& key, std::unique_ptr<Expr> value = nullptr)
      : Expr(NodeType::Property), key(key), value(std::move(value)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "Property: " << key.value;
    if (value) {
      oss << " = " << value->toString();
    }
    return oss.str();
  }
};

class ObjectLiteral : public Expr {
public:
  std::vector<std::unique_ptr<Property>> properties;

  explicit ObjectLiteral(std::vector<std::unique_ptr<Property>> properties)
      : Expr(NodeType::ObjectLiteral), properties(std::move(properties)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "ObjectLiteral: { ";
    for (size_t i = 0; i < properties.size(); ++i) {
      oss << properties[i]->toString();
      if (i < properties.size() - 1) {
        oss << ", ";
      }
    }
    oss << " }";
    return oss.str();
  }
};

class ArrayLiteral : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> elements;

  explicit ArrayLiteral(std::vector<std::unique_ptr<Expr>> elems)
      : Expr(NodeType::ArrayLiteral), elements(std::move(elems)) {}

  std::string toString() const override {
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); ++i) {
      result += elements[i]->toString();
      if (i < elements.size() - 1) {
        result += ", ";
      }
    }
    result += "]";
    return result;
  }
};

class Identifier : public Expr {
public:
  Token identifier;

  explicit Identifier(const Token& identifier) : Expr(NodeType::Identifier), identifier(identifier) {}

  std::string toString() const override { return "Identifier: " + identifier.value; }
};

class NullLiteral : public Expr {
public:
  explicit NullLiteral() : Expr(NodeType::NullLiteral) {}

  std::string toString() const override { return "NullLiteral"; }
};

class IntegerLiteral : public Expr {
public:
  int value;

  explicit IntegerLiteral(int value) : Expr(NodeType::IntegerLiteral), value(value) {}

  std::string toString() const override { return "IntegerLiteral: " + std::to_string(value); }
};

class DoubleLiteral : public Expr {
public:
  double value;

  explicit DoubleLiteral(double value) : Expr(NodeType::DoubleLiteral), value(value) {}

  std::string toString() const override { return "DoubleLiteral: " + std::to_string(value); }
};

class BooleanLiteral : public Expr {
public:
  bool value;

  explicit BooleanLiteral(bool value) : Expr(NodeType::BooleanLiteral), value(value) {}

  std::string toString() const override { return "BooleanLiteral: " + std::to_string(value); }
};

class CharLiteral : public Expr {
public:
  char value;

  explicit CharLiteral(char value) : Expr(NodeType::CharLiteral), value(value) {}

  std::string toString() const override { return "CharLiteral: '" + std::string(1, value) + "'"; }
};

class StringLiteral : public Expr {
public:
  std::string value;

  explicit StringLiteral(std::string value) : Expr(NodeType::StringLiteral), value(std::move(value)) {}

  std::string toString() const override { return "StringLiteral: \"" + value + "\""; }
};

class HexCodeLiteral : public Expr {
public:
  std::string value;

  explicit HexCodeLiteral(std::string value) : Expr(NodeType::HexCodeLiteral), value(std::move(value)) {}

  std::string toString() const override { return "HexCodeLiteral: " + value; }
};

// Short Notation
class ShortOperationLiteral : public Expr {
public:
  Token type;
  Token operation_;
  std::unique_ptr<Expr> value;

  ShortOperationLiteral(const Token& type, const Token& operation_, std::unique_ptr<Expr> value)
      : Expr(NodeType::ShortOperationLiteral), type(type), operation_(operation_), value(std::move(value)) {}

  std::string toString() const override {
    return "ShortOperationLiteral: " + type.plainText + "(" + operation_.value + value->toString() + ")";
  }
};

class ShortSingleExpressionLiteral : public Expr {
public:
  Token type;
  std::unique_ptr<Expr> value;

  ShortSingleExpressionLiteral(const Token& type, std::unique_ptr<Expr> value)
      : Expr(NodeType::ShortSingleExpressionLiteral), type(type), value(std::move(value)) {}

  std::string toString() const override {
    return "ShortSingleExpressionLiteral: " + type.plainText + "(" + value->toString() + ")";
  }
};

class ShortDoubleExpressionLiteral : public Expr {
public:
  Token type;
  std::unique_ptr<Expr> value1;
  std::unique_ptr<Expr> value2;

  ShortDoubleExpressionLiteral(const Token& type, std::unique_ptr<Expr> value1, std::unique_ptr<Expr> value2)
      : Expr(NodeType::ShortDoubleExpressionLiteral),
        type(type),
        value1(std::move(value1)),
        value2(std::move(value2)) {}

  std::string toString() const override {
    return "ShortDoubleExpressionLiteral: " + type.plainText + "(" + value1->toString() + ", " + value2->toString() +
           ")";
  }
};

#endif  // AST_NODES_HPP
