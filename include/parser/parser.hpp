#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lexer/lexer.hpp"
#include "parser/ast_nodes.hpp"

class Parser {
public:
  Parser(const std::vector<Token>& tokens);
  std::unique_ptr<Program> parse(bool devMode);

  std::vector<Token> tokens;

private:
  void handleException(const std::exception& e);
  std::unique_ptr<Program> buildAST();
  std::unique_ptr<Stmt> parseStatement();
  std::vector<std::unique_ptr<Stmt>> parseStatementBlock();
  std::unique_ptr<Stmt> parseFunctionDeclaration();
  std::unique_ptr<Stmt> parseVarDeclaration();
  std::unique_ptr<Stmt> parseConditionalStatement();
  std::unique_ptr<Stmt> parseWhileStatement();
  std::unique_ptr<Stmt> parseDoWhileStatement();
  std::unique_ptr<Stmt> parseForStatement();
  std::unique_ptr<Stmt> parseReturnStatement();
  std::unique_ptr<Expr> parseExpression();
  std::unique_ptr<Expr> parseAssignmentExpr();
  std::unique_ptr<Expr> parseTernaryExpr();
  std::unique_ptr<Expr> parseLogicalExpr();
  std::unique_ptr<Expr> parseEqualityExpr();
  std::unique_ptr<Expr> parseRelationalExpr();
  std::unique_ptr<Expr> parseObjectExpr();
  std::unique_ptr<Expr> parseArrayExpr();
  std::unique_ptr<Expr> parseAdditiveExpr();
  std::unique_ptr<Expr> parseMultiplicativeExpr();
  std::unique_ptr<Expr> parsePowerExpr();
  std::unique_ptr<Expr> parseCallMemberExpr();
  std::unique_ptr<Expr> parseCallExpr(std::unique_ptr<Expr> caller);
  std::vector<std::unique_ptr<Expr>> parseArgs();
  std::vector<std::unique_ptr<Expr>> parseArgumentsList();
  std::unique_ptr<Expr> parseMemberExpr();
  std::unique_ptr<Expr> parsePrimaryExpr();
  void parseComment();
  std::unique_ptr<Stmt> parseLogStatement();

  bool isEOF();
  Token peek();
  Token advance();
  Token assertToken(const std::string& expected, const std::string& errorMessage);
  void log(const std::unique_ptr<Program>& program, bool devMode);
  void skipSemicolon();
};

#endif
