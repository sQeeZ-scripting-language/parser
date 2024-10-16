#include "parser/parser.hpp"

Parser::Parser(const std::string& code) : code(code) {}

void Parser::parse(bool devMode) {
  Lexer lexer(code);
  tokens = lexer.tokenize(devMode);
  assertToken("BasicToken::INIT");

  while (!tokens.empty()) {
    std::cout << parseAdditiveExpression().get()->toString() << std::endl;
  }
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
  return parseAssignmentExpression();
}

std::unique_ptr<ExpressionNode> Parser::parseAssignmentExpression() {
  auto left = parseObjectExpression();

  if (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::ASSIGN) {
    advance();
    auto right = parseAssignmentExpression();
    return std::make_unique<AssignmentExpressionNode>(std::move(left), std::move(right));
  }

  return left;
}

std::unique_ptr<ExpressionNode> Parser::parseObjectExpression() {
  if (peek().tag != Token::TypeTag::SYNTAX || peek().type.syntaxToken != SyntaxToken::OPEN_BRACE) {
    return parseAdditiveExpression();
  }

  advance();
  std::vector<std::unique_ptr<ExpressionNode>> properties;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    auto key = assertToken("DataToken::IDENTIFIER");
    if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
      advance();
      properties.push_back(std::make_unique<PropertyNode>(nullptr, key));
      continue;
    } else if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE) {
      properties.push_back(std::make_unique<PropertyNode>(nullptr, key));
      continue;
    }

    assertToken("SyntaxToken::COLON");
    auto value = parseExpression();

    properties.push_back(std::make_unique<PropertyNode>(std::move(value), key));
    if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
      assertToken("SyntaxToken::COMMA");
    }
  }

  assertToken("SyntaxToken::CLOSE_BRACE");
  return std::make_unique<ObjectLiteralNode>(std::move(properties));
}

std::unique_ptr<ExpressionNode> Parser::parseAdditiveExpression() {
  auto left = parseMultiplicativeExpression();

  while (peek().value == "+" || peek().value == "-") {
    Token op = advance();
    auto right = parseMultiplicativeExpression();
    left = std::make_unique<BinaryExpressionNode>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicativeExpression() {
  auto left = parseCallMemberExpression();

  while (peek().value == "*" || peek().value == "/" || peek().value == "%") {
    Token op = advance();
    auto right = parseCallMemberExpression();
    left = std::make_unique<BinaryExpressionNode>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<ExpressionNode> Parser::parseCallMemberExpression() {
  auto member = parseMemberExpression();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    return parseCallExpression(std::move(member));
  }

  return member;
}

std::unique_ptr<ExpressionNode> Parser::parseCallExpression(std::unique_ptr<ExpressionNode> caller) {
  auto callExpr = std::make_unique<CallExpressionNode>(std::move(caller), parseArgs());

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    return parseCallExpression(std::move(callExpr));
  }

  return callExpr;
}

std::vector<std::unique_ptr<ExpressionNode>> Parser::parseArgs() {
  assertToken("SyntaxToken::OPEN_PARENTHESIS");
  auto args = peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_PARENTHESIS ? std::vector<std::unique_ptr<ExpressionNode>>() : parseArgumentsList();
  assertToken("SyntaxToken::CLOSE_PARENTHESIS");
  return args;
}

std::vector<std::unique_ptr<ExpressionNode>> Parser::parseArgumentsList() {
  auto args = std::vector<std::unique_ptr<ExpressionNode>>();
  args.push_back(parseAssignmentExpression());

  while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
    advance();
    args.push_back(parseAssignmentExpression());
  }

  return args;
}

std::unique_ptr<ExpressionNode> Parser::parseMemberExpression() {
  auto object = parsePrimaryExpression();

  while (peek().tag == Token::TypeTag::SYNTAX && (peek().type.syntaxToken == SyntaxToken::DOT || peek().type.syntaxToken == SyntaxToken::OPEN_BRACKET)) {
    Token op = advance();
    std::unique_ptr<ExpressionNode> property;
    bool computed;
    
    if (op.tag == Token::TypeTag::SYNTAX && op.type.syntaxToken == SyntaxToken::DOT) {
      computed = false;
      property = parsePrimaryExpression();
      auto primaryExpr = std::make_unique<PrimaryExpressionNode>(property);
    
      if (!(primaryExpr->getToken().tag == Token::TypeTag::DATA && primaryExpr->getToken().type.dataToken == DataToken::IDENTIFIER)) {
        std::cerr << "Cannot use dot operator without right hand side being an identifier" << std::endl;
        assert(false);
      }

    } else {
      computed = true;
      property = parseExpression();
      assertToken("SyntaxToken::CLOSE_BRACKET");
    }

    object = std::make_unique<MemberExpressionNode>(std::move(object), std::move(property), computed);
  }

  return object;
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
  } else if (token.tag == Token::TypeTag::SYNTAX && token.type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    auto expression = parseExpression();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS");
    return expression;
  }

  std::cerr << "Unexpected token: " << token.plainText << std::endl;
  assert(false);
  return nullptr;
}

bool Parser::isEOF() {
  return peek().tag == Token::TypeTag::BASIC && peek().type.basicToken == BasicToken::TOKEN_EOF;
}

Token Parser::peek() {
  return tokens.front();
}

Token Parser::advance() {
  Token token = tokens.front();
  tokens.erase(tokens.begin());
  return token;
}

Token Parser::assertToken(std::string expected) {
  Token token = advance();
  if (token.plainText != expected) {
    std::cerr << "Assertion failed: Expected '" << expected << "', but received '" << token.plainText << "'"
              << std::endl;
    assert(false);
  }
  return token;
}