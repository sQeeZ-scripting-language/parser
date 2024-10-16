#include "parser/parser.hpp"

Parser::Parser(const std::string& code) : code(code) {}

void Parser::parse(bool devMode) {
  Lexer lexer(code);
  tokens = lexer.tokenize(devMode);
  assertToken(advance(), "BasicToken::INIT");

  while (!tokens.empty()) {
    Token token = advance();
    if (token.tag == Token::TypeTag::BASIC && token.type.basicToken == BasicToken::TOKEN_EOF) {
      break;
    }
    parsePrimaryExpression(token);
  }
}

PrimaryExpressionNode Parser::parsePrimaryExpression(Token token) {
  if (token.tag == Token::TypeTag::DATA) {
    switch (token.type.dataToken) {
      case DataToken::INTEGER_LITERAL:
      case DataToken::DOUBLE_LITERAL:
      case DataToken::STRING_LITERAL:
      case DataToken::IDENTIFIER:
        return PrimaryExpressionNode(token);
      default:
        break;
    }
  }
  std::cerr << "Unexpected token: " << token.plainText << std::endl;
  assert(false);
}

Token Parser::advance() {
  Token token = tokens.front();
  tokens.erase(tokens.begin());
  return token;
}

void Parser::assertToken(Token token, std::string expected) {
  if (token.plainText != expected) {
    std::cerr << "Assertion failed: Expected '" << expected << "', but received '" << token.plainText << "'"
              << std::endl;
    assert(false);
  }
}