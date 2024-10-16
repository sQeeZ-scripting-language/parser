#include "parser/parser.hpp"

Parser::Parser(const std::string& code) : code(code) {}

void Parser::parse(bool devMode) {
  Lexer lexer(code);
  tokens = lexer.tokenize(devMode);

  while (!tokens.empty()) {
    Token token = advance();
    assertToken(token, "BasicToken::INIT");
  }
}

Token Parser::advance() {
  Token token = tokens.front();
  tokens.erase(tokens.begin());
  return token;
}

void Parser::assertToken(Token token, std::string expected) {
  if (token.plainText != expected) {
    std::cerr << "Assertion failed: Expected '" << expected << "', but received '" << token.plainText << "'" << std::endl;
    assert(false);
  }
}