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

  PrimaryExpressionNode parsePrimaryExpression();

  Token advance();
  void assertToken(Token token, std::string expected);
};

#endif
