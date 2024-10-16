#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <cassert>

#include "lexer/lexer.hpp"

class Parser {
public:
  Parser(const std::string& code);
  void parse(bool devMode);

  const std::string code;

private:
  std::vector<Token> tokens;

  Token advance();
  void assertToken(Token token, std::string expected);
};

#endif
