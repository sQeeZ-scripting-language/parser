#include "parser/parser.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<Program> Parser::parse(bool devMode) {
  assertToken("BasicToken::INIT", "Expected INIT token at the beginning of the token stream.");
  std::unique_ptr<Program> ast = buildAST();
  log(ast, devMode);
  return std::move(ast);
}

std::unique_ptr<Program> Parser::buildAST() {
  auto program = std::make_unique<Program>();
  program->kind = NodeType::Program;

  while (!isEOF()) {
    auto statement = parseStatement();
    if (statement != nullptr) {
      program->body.push_back(std::move(statement));
    }
  }

  return program;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (peek().tag == Token::TypeTag::KEYWORD) {
    switch (peek().type.keywordToken) {
      case KeywordToken::CONSTANT:
      case KeywordToken::VARIABLE:
        return parseVarDeclaration();
        return parseVarDeclaration();
      case KeywordToken::FUNCTION:
        return parseFunctionDeclaration();
      default:
        return parseExpression();
    }
  } else if (peek().tag == Token::TypeTag::LOG) {
    return parseLogStatement();
  } else if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::INLINE_COMMENT) {
    parseComment();
    return nullptr;
  } else {
    return parseExpression();
  }
}

std::unique_ptr<Stmt> Parser::parseFunctionDeclaration() {
  advance();
  std::string name = assertToken("DataToken::IDENTIFIER", "Expected function name following fn keyword").value;

  auto args = parseArgs();
  std::vector<std::string> params;

  for (const auto& arg : args) {
    if (arg->kind != NodeType::Identifier) {
      std::cerr << arg.get() << std::endl;
      throw std::runtime_error("Inside function declaration expected parameters to be of type string.");
    }
    params.push_back(static_cast<Identifier*>(arg.get())->symbol);
  }

  assertToken("SyntaxToken::OPEN_BRACE", "Expected function body following declaration");
  std::vector<std::unique_ptr<Stmt>> body;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    body.push_back(parseStatement());
  }

  assertToken("SyntaxToken::CLOSE_BRACE", "Closing brace expected inside function declaration");

  auto fn = std::make_unique<FunctionDeclaration>(name, std::move(params), std::move(body));

  return fn;
}

std::unique_ptr<Stmt> Parser::parseVarDeclaration() {
  bool isConstant = (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::CONSTANT);
  advance();
  std::string identifier =
      assertToken("DataToken::IDENTIFIER", "Expected identifier name following var | const keywords.").value;

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::SEMICOLON) {
    advance();
    if (isConstant) {
      throw std::runtime_error("Must assign value to constant expression. No value provided.");
    }

    return std::make_unique<VarDeclaration>(false, identifier, nullptr);
  }

  assertToken("OperatorToken::ASSIGN", "Expected assign token following identifier in var declaration.");
  std::unique_ptr<Expr> value = parseExpression();
  assertToken("SyntaxToken::SEMICOLON", "Variable declaration statment must end with semicolon.");

  return std::make_unique<VarDeclaration>(isConstant, identifier, std::move(value));
}

std::unique_ptr<Expr> Parser::parseExpression() { return parseAssignmentExpr(); }

std::unique_ptr<Expr> Parser::parseAssignmentExpr() {
  auto left = parseObjectExpr();

  if (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::ASSIGN) {
    advance();
    auto value = parseAssignmentExpr();
    return std::make_unique<AssignmentExpr>(std::move(left), std::move(value));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseObjectExpr() {
  if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACE)) {
    return parseAdditiveExpr();
  }

  advance();
  std::vector<std::unique_ptr<Property>> properties;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    std::string key = assertToken("DataToken::Identifier", "Object literal key expected").value;

    // pair -> { key, }
    if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
      advance();
      properties.push_back(std::make_unique<Property>(Property{key, nullptr}));
      continue;
    }
    // pair -> { key }
    else if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE) {
      properties.push_back(std::make_unique<Property>(Property{key, nullptr}));
      continue;
    }

    // { key: val }
    assertToken("SyntaxToken::COLON", "Missing colon following identifier in ObjectExpr");
    auto value = parseExpression();

    properties.push_back(std::make_unique<Property>(Property{key, std::move(value)}));
    if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
      assertToken("SyntaxToken::COMMA", "Expected comma or closing bracket following property");
    }
  }

  assertToken("SyntaxToken::CLOSE_BRACE", "Object literal missing closing brace.");
  return std::make_unique<ObjectLiteral>(std::move(properties));
}

std::unique_ptr<Expr> Parser::parseAdditiveExpr() {
  auto left = parseMultiplicativeExpr();

  while (peek().value == "+" || peek().value == "-") {
    std::string op = advance().value;
    auto right = parseMultiplicativeExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicativeExpr() {
  auto left = parsePowerExpr();

  while (peek().value == "/" || peek().value == "*" || peek().value == "%") {
    std::string op = advance().value;
    auto right = parsePowerExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parsePowerExpr() {
  auto left = parseCallMemberExpr();

  while (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::POTENTIATION) {
    std::string op = advance().value;
    auto right = parseCallMemberExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseCallMemberExpr() {
  auto member = parseMemberExpr();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    return parseCallExpr(std::move(member));
  }

  return member;
}

std::unique_ptr<Expr> Parser::parseCallExpr(std::unique_ptr<Expr> caller) {
  auto callExpr = std::make_unique<CallExpr>();
  callExpr->caller = std::move(caller);
  callExpr->args = parseArgs();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    callExpr = std::unique_ptr<CallExpr>(static_cast<CallExpr*>(parseCallExpr(std::move(callExpr)).release()));
  }

  return callExpr;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgs() {
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected open parenthesis");
  std::vector<std::unique_ptr<Expr>> args;

  if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_PARENTHESIS)) {
    args = parseArgumentsList();
  }

  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Missing closing parenthesis inside arguments list");
  return args;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgumentsList() {
  std::vector<std::unique_ptr<Expr>> args;
  args.push_back(parseAssignmentExpr());

  while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
    advance();
    args.push_back(parseAssignmentExpr());
  }

  return args;
}

std::unique_ptr<Expr> Parser::parseMemberExpr() {
  std::unique_ptr<Expr> object = parsePrimaryExpr();

  while ((peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::DOT) ||
         (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACKET)) {
    Token operatorToken = advance();
    std::unique_ptr<Expr> property;
    bool computed;

    // dot notation
    if (operatorToken.type.syntaxToken == SyntaxToken::DOT) {
      computed = false;
      property = parsePrimaryExpr();
      if (property->kind != NodeType::Identifier) {
        throw std::runtime_error("Cannot use dot operator without right-hand side being an identifier.");
      }
    }
    // bracket notation
    else {
      computed = true;
      property = parseExpression();
      assertToken("SyntaxToken::CLOSE_BRACKET", "Missing closing bracket in computed value.");
    }

    object = std::make_unique<MemberExpr>(MemberExpr{std::move(object), std::move(property), computed});
  }

  return object;
}

std::unique_ptr<Expr> Parser::parsePrimaryExpr() {
  Token token = peek();

  if (token.tag == Token::TypeTag::DATA) {
    switch (token.type.dataToken) {
      case DataToken::INTEGER_LITERAL:
        return std::make_unique<IntegerLiteral>(IntegerLiteral{std::stoi(advance().value)});
      case DataToken::DOUBLE_LITERAL:
        return std::make_unique<DoubleLiteral>(DoubleLiteral{std::stod(advance().value)});
      case DataToken::IDENTIFIER:
        return std::make_unique<Identifier>(Identifier{advance().value});
      default:
        break;
    }
  } else if (token.tag == Token::TypeTag::SYNTAX) {
    std::unique_ptr<Expr> expression;
    std::string value;
    switch (token.type.syntaxToken) {
      case SyntaxToken::OPEN_PARENTHESIS:
        advance();
        expression = parseExpression();
        assertToken("SyntaxToken::CLOSE_PARENTHESIS",
                    "Unexpected token found inside parenthesised expression. Expected closing parenthesis.");
        return expression;
      case SyntaxToken::DOUBLE_QUOTE:
        advance();
        value = assertToken("DataToken::STRING_LITERAL", "Expected string literal").value;
        assertToken("SyntaxToken::DOUBLE_QUOTE", "Expected closing double quote");
        return std::make_unique<StringLiteral>(StringLiteral{value});
      case SyntaxToken::HASHTAG:
        advance();
        value = assertToken("DataToken::HEX_CODE_LITERAL", "Expected hex code literal").value;
        return std::make_unique<HexCodeLiteral>(HexCodeLiteral{"#" + value});
      case SyntaxToken::INLINE_COMMENT:
        parseComment();
        return nullptr;
      default:
        break;
    }
  }

  std::cerr << "Unexpected token: " << token.plainText << std::endl;
  assert(false);
  return nullptr;
}

void Parser::parseComment() {
  assertToken("SyntaxToken::INLINE_COMMENT", "Expected inline comment token");
  if (peek().tag == Token::TypeTag::DATA && peek().type.dataToken == DataToken::COMMENT_LITERAL) {
    advance();
  }
}

std::unique_ptr<Stmt> Parser::parseLogStatement() {
  Token logType = advance();
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after log function call.");

  auto messageExpr = parseExpression();

  std::unique_ptr<Expr> colorExpr = nullptr;
  if (logType.tag == Token::TypeTag::LOG && logType.type.logToken == LogToken::COLORED) {
    assertToken("SyntaxToken::COMMA", "Expected ',' between message and color in logc.");
    colorExpr = parseExpression();
  }

  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after log function call.");
  assertToken("SyntaxToken::SEMICOLON", "Expected ';' after log function call.");
  return std::make_unique<LogStmt>(logType, std::move(messageExpr), std::move(colorExpr));
}

bool Parser::isEOF() { return peek().tag == Token::TypeTag::BASIC && peek().type.basicToken == BasicToken::TOKEN_EOF; }

Token Parser::peek() { return tokens.front(); }

Token Parser::advance() {
  Token token = tokens.front();
  tokens.erase(tokens.begin());
  return token;
}

Token Parser::assertToken(const std::string& expected, const std::string& errorMessage) {
  Token token = advance();
  if (token.plainText != expected) {
    std::cerr << errorMessage << std::endl;
    assert(false);
  }
  return token;
}

void Parser::log(const std::unique_ptr<Program>& program, bool devMode) {
  if (devMode) {
    std::cout << program->toString() << std::endl;
  }
}