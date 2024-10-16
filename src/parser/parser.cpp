#include "parser/parser.hpp"

Parser::Parser(const std::string& code) : code(code) {}

void Parser::parse(bool devMode) {
  Lexer lexer(code);
  tokens = lexer.tokenize(devMode);
  assertToken(advance(), "BasicToken::INIT");

  while (!tokens.empty()) {
    parsePrimaryExpression();
  }
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicativeExpression() {
  auto left = parsePrimaryExpression();

  while (peek().value == "*" || peek().value == "/") {
    Token op = advance();
    auto right = parsePrimaryExpression();
    left = std::make_unique<BinaryExpressionNode>(std::move(left), std::move(right), op);
  }
  return left;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimaryExpression() {
  Token token = advance();

  if (token.tag == Token::TypeTag::BASIC && token.type.basicToken == BasicToken::TOKEN_EOF) {
    return std::make_unique<PrimaryExpressionNode>(token);
  } else if (token.tag == Token::TypeTag::DATA) {
    switch (token.type.dataToken) {
      case DataToken::INTEGER_LITERAL:
      case DataToken::DOUBLE_LITERAL:
      case DataToken::STRING_LITERAL:
      case DataToken::IDENTIFIER:
        return std::make_unique<PrimaryExpressionNode>(token);
      default:
        break;
    }
  }

  std::cerr << "Unexpected token: " << token.plainText << std::endl;
  assert(false);
  return nullptr;
}

Token Parser::peek() {
  return tokens.front();
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