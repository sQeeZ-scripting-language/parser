#include "parser/parser.hpp"

Parser::Parser(const std::string& code) : code(code) {}

void Parser::parse(bool devMode) {
  Lexer lexer(code);
  std::vector<Token> tokens = lexer.tokenize(devMode);
}