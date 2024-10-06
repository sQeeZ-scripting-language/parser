#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>

#include "lexer/lexer.hpp"

class Parser {
public:
  Parser(const std::string& code);
  void parse(bool devMode);

  const std::string code;
};

#endif
