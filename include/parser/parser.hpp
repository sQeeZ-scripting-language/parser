#ifndef PARSER_HPP
#define PARSER_HPP

#include <cassert>
#include <iostream>

#include "parser/nodes/expression_node.hpp"
#include "lexer/lexer.hpp"

class Parser {
public:
  Parser(const std::string& code);
  void parse(bool devMode);

  const std::string code;

private:
  std::vector<Token> tokens;

  std::unique_ptr<ExpressionNode> parseExpression();
  std::unique_ptr<ExpressionNode> parseAssignmentExpression();
  std::unique_ptr<ExpressionNode> parseObjectExpression();
  std::unique_ptr<ExpressionNode> parseAdditiveExpression();
  std::unique_ptr<ExpressionNode> parseMultiplicativeExpression();
  std::unique_ptr<ExpressionNode> parsePrimaryExpression();

  bool isEOF();
  Token peek();
  Token advance();
  Token Parser::assertToken(std::string expected);
};

#endif
