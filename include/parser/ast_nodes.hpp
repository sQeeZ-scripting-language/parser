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
  VarDeclaration,
  FunctionDeclaration,
  ConditionalStatement,
  LogStmt,

  // EXPRESSIONS
  AssignmentExpr,
  MemberExpr,
  CallExpr,

  // LITERALS
  Property,
  ObjectLiteral,
  IntegerLiteral,
  DoubleLiteral,
  StringLiteral,
  HexCodeLiteral,
  Identifier,
  BinaryExpr
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

class VarDeclaration : public Stmt {
public:
  bool constant;
  std::string identifier;
  std::unique_ptr<ASTNode> value;

  VarDeclaration(bool constant, const std::string& identifier, std::unique_ptr<ASTNode> value)
      : Stmt(NodeType::VarDeclaration), constant(constant), identifier(identifier), value(std::move(value)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "VarDeclaration: " << (constant ? "const " : "let ") << identifier;
    if (value) {
      oss << " = " << value->toString();
    }
    return oss.str();
  }
};

class FunctionDeclaration : public Stmt {
public:
  std::string name;
  std::vector<std::string> parameters;
  std::vector<std::unique_ptr<Stmt>> body;

  FunctionDeclaration(const std::string& name, std::vector<std::string> parameters,
                      std::vector<std::unique_ptr<Stmt>> body)
      : Stmt(NodeType::FunctionDeclaration), name(name), parameters(std::move(parameters)), body(std::move(body)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "FunctionDeclaration: " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
      oss << parameters[i];
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

class BinaryExpr : public Expr {
public:
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  std::string operator_;

  BinaryExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, const std::string& operator_)
      : Expr(NodeType::BinaryExpr), left(std::move(left)), right(std::move(right)), operator_(operator_) {}

  std::string toString() const override {
    return "BinaryExpr: (" + left->toString() + " " + operator_ + " " + right->toString() + ")";
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

// Literal / Primary Expressions
class Identifier : public Expr {
public:
  std::string symbol;

  explicit Identifier(const std::string& symbol) : Expr(NodeType::Identifier), symbol(symbol) {}

  std::string toString() const override { return "Identifier: " + symbol; }
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

class Property : public Expr {
public:
  std::string key;
  std::unique_ptr<Expr> value;

  Property(const std::string& key, std::unique_ptr<Expr> value = nullptr)
      : Expr(NodeType::Property), key(key), value(std::move(value)) {}

  std::string toString() const override {
    std::ostringstream oss;
    oss << "Property: " << key;
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

#endif
