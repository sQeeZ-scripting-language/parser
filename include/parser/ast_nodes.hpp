#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

enum class NodeType {
    // STATEMENTS
    Program,
    VarDeclaration,
    FunctionDeclaration,
    
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
        return "VarDeclaration: " + identifier;
    }
};

class FunctionDeclaration : public Stmt {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<Stmt>> body;

    FunctionDeclaration(const std::string& name, std::vector<std::string> parameters, std::vector<std::unique_ptr<Stmt>> body)
        : Stmt(NodeType::FunctionDeclaration), name(name), parameters(parameters), body(std::move(body)) {}

    std::string toString() const override {
        return "FunctionDeclaration: " + name;
    }
};

// Expressions (result in a runtime value)
class Expr : public Stmt {
public:
    explicit Expr(const NodeType& kind) : Stmt(kind) {}
};

class AssignmentExpr : public Expr {
public:
    std::unique_ptr<Expr> assignee;
    std::unique_ptr<Expr> value;

    AssignmentExpr(std::unique_ptr<Expr> assignee, std::unique_ptr<Expr> value)
        : Expr(NodeType::AssignmentExpr), assignee(std::move(assignee)), value(std::move(value)) {}

    std::string toString() const override {
        return "AssignmentExpr";
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
        return "BinaryExpr: " + operator_;
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
        return "CallExpr";
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
        return "MemberExpr";
    }
};

// Literal / Primary Expressions
class Identifier : public Expr {
public:
    std::string symbol;

    explicit Identifier(const std::string& symbol)
        : Expr(NodeType::Identifier), symbol(symbol) {}

    std::string toString() const override {
        return "Identifier: " + symbol;
    }
};

class IntegerLiteral : public Expr {
public:
    int value;

    explicit IntegerLiteral(int value)
        : Expr(NodeType::IntegerLiteral), value(value) {}

    std::string toString() const override {
        return "IntegerLiteral: " + std::to_string(value);
    }
};

class DoubleLiteral : public Expr {
public:
    double value;

    explicit DoubleLiteral(double value)
        : Expr(NodeType::DoubleLiteral), value(value) {}

    std::string toString() const override {
        return "DoubleLiteral: " + std::to_string(value);
    }
};

class StringLiteral : public Expr {
public:
    std::string value;

    explicit StringLiteral(std::string value)
        : Expr(NodeType::StringLiteral), value(value) {}

    std::string toString() const override {
        return "StringLiteral: " + value;
    }
};

class Property : public Expr {
public:
    std::string key;
    std::unique_ptr<Expr> value;

    Property(const std::string& key, std::unique_ptr<Expr> value = nullptr)
        : Expr(NodeType::Property), key(key), value(std::move(value)) {}

    std::string toString() const override {
        return "Property: " + key;
    }
};

class ObjectLiteral : public Expr {
public:
    std::vector<std::unique_ptr<Property>> properties;

    explicit ObjectLiteral(std::vector<std::unique_ptr<Property>> properties)
        : Expr(NodeType::ObjectLiteral), properties(std::move(properties)) {}

    std::string toString() const override {
        return "ObjectLiteral";
    }
};

#endif
