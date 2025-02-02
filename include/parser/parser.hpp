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
  // Statements
  std::unique_ptr<Stmt> parseStatement();
  std::vector<std::unique_ptr<Stmt>> parseStatementBlock();
  std::unique_ptr<Stmt> parseFunctionDeclaration();
  std::unique_ptr<Stmt> parseReturnStatement();
  std::unique_ptr<Stmt> parseVarDeclaration();
  std::unique_ptr<Stmt> parseConditionalStatement();
  std::unique_ptr<Stmt> parseWhileStatement();
  std::unique_ptr<Stmt> parseDoWhileStatement();
  std::unique_ptr<Stmt> parseForStatement();
  std::unique_ptr<Stmt> parseLogStatement();
  // Expressions
  std::unique_ptr<Expr> parseExpression();
  std::unique_ptr<Expr> parseAssignmentExpr();
  std::unique_ptr<Expr> parseTernaryExpr();
  std::unique_ptr<Expr> parseLogicalExpr();
  std::unique_ptr<Expr> parseEqualityExpr();
  std::unique_ptr<Expr> parseRelationalExpr();
  std::unique_ptr<Expr> parseObjectExpr();
  std::unique_ptr<Expr> parseArrayExpr();
  std::unique_ptr<Expr> parseCallbackFunctionExpr();
  std::unique_ptr<Expr> parseShortData();
  std::unique_ptr<Expr> parseAdditiveExpr();
  std::unique_ptr<Expr> parseMultiplicativeExpr();
  std::unique_ptr<Expr> parsePowerExpr();
  std::unique_ptr<Expr> parseCallMemberExpr();
  std::unique_ptr<Expr> parseCallExpr(std::unique_ptr<Expr> caller, std::unique_ptr<Expr> method);
  std::unique_ptr<Expr> parseShortExpr(std::unique_ptr<Expr> caller, std::unique_ptr<Expr> method);
  std::vector<std::unique_ptr<Expr>> parseArgs();
  std::vector<std::unique_ptr<Expr>> parseShortArgs();
  std::vector<std::unique_ptr<Expr>> parseArgumentsList();
  std::unique_ptr<Expr> parseMemberExpr();
  std::unique_ptr<Expr> parsePrimaryExpr();

  // Utility functions
  bool isEOF();
  Token peek(int steps = 1);
  Token lookAhead(int steps);
  Token advance();
  Token assertToken(const std::string& expected, const std::string& errorMessage);
  void log(const std::unique_ptr<Program>& program, bool devMode);
  void skipSemicolon();
  void skipComment();
  inline static const std::unordered_map<ShortNotationToken, std::string> shortEnumToString = {
      {ShortNotationToken::LENGTH, "length"},
      {ShortNotationToken::CONCAT, "concat"},
      {ShortNotationToken::INCLUDES, "includes"},
      {ShortNotationToken::INDEX_OF, "indexOf"},
      {ShortNotationToken::LAST_INDEX_OF, "lastIndexOf"},
      {ShortNotationToken::SLICE, "slice"},
      {ShortNotationToken::PUSH, "push"},
      {ShortNotationToken::POP, "pop"},
      {ShortNotationToken::SHIFT, "shift"},
      {ShortNotationToken::UNSHIFT, "unshift"},
      {ShortNotationToken::SPLICE, "splice"},
      {ShortNotationToken::REVERSE, "reverse"},
      {ShortNotationToken::SORT, "sort"},
      {ShortNotationToken::FILL, "fill"},
      {ShortNotationToken::JOIN, "join"},
      {ShortNotationToken::COUNT, "count"},
      {ShortNotationToken::EVERY, "every"},
      {ShortNotationToken::SOME, "some"},
      {ShortNotationToken::FIND, "find"},
      {ShortNotationToken::FIND_INDEX, "findIndex"},
      {ShortNotationToken::FIND_LAST, "findLast"},
      {ShortNotationToken::FIND_LAST_INDEX, "findLastIndex"},
      {ShortNotationToken::FILTER, "filter"},
      {ShortNotationToken::MAP, "map"},
      {ShortNotationToken::REDUCE, "reduce"},
      {ShortNotationToken::FLAT, "flat"},
      {ShortNotationToken::FLAT_MAP, "flatMap"},
      {ShortNotationToken::FOR_EACH, "forEach"},
      {ShortNotationToken::HAS_KEY, "hasKey"},
      {ShortNotationToken::KEYS, "keys"},
      {ShortNotationToken::VALUES, "values"},
      {ShortNotationToken::ENTRIES, "entries"},
      {ShortNotationToken::GET, "get"},
      {ShortNotationToken::CHAR_AT, "charAt"},
      {ShortNotationToken::CHAR_CODE_AT, "charCodeAt"},
      {ShortNotationToken::MATCH, "match"},
      {ShortNotationToken::MATCH_ALL, "matchAll"},
      {ShortNotationToken::PAD_END, "padEnd"},
      {ShortNotationToken::PAD_START, "padStart"},
      {ShortNotationToken::REPEAT, "repeat"},
      {ShortNotationToken::REPLACE, "replace"},
      {ShortNotationToken::REPLACE_ALL, "replaceAll"},
      {ShortNotationToken::SPLIT, "split"},
      {ShortNotationToken::STARTS_WITH, "startsWith"},
      {ShortNotationToken::ENDS_WITH, "endsWith"},
      {ShortNotationToken::SUBSTRING, "substring"},
      {ShortNotationToken::LOWERCASE, "lowercase"},
      {ShortNotationToken::UPPERCASE, "uppercase"},
      {ShortNotationToken::TRIM, "trim"},
      {ShortNotationToken::TRIM_END, "trimEnd"},
      {ShortNotationToken::TRIM_START, "trimStart"}};
};

#endif  // PARSER_HPP
