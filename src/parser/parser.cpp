#include "parser/parser.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<Program> Parser::parse(bool devMode) {
  assertToken("BasicToken::INIT", "Expected INIT token at the beginning of the token stream.");
  std::unique_ptr<Program> ast = buildAST();
  log(ast, devMode);
  return std::move(ast);
}

void Parser::handleException(const std::exception& e) {
  std::string exceptionType = typeid(e).name();

  std::cerr << "\033[1;30m\033[41m[sQeeZ]: "
            << "Exception of type: " << exceptionType << " - Message: " << e.what() << "\033[0m" << std::endl;

  std::exit(EXIT_FAILURE);
}

std::unique_ptr<Program> Parser::buildAST() {
  auto program = std::make_unique<Program>();
  program->kind = NodeType::Program;

  try {
    while (!isEOF()) {
      auto statement = parseStatement();
      if (statement != nullptr) {
        program->body.push_back(std::move(statement));
      }
      skipSemicolon();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
  return program;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (peek().tag == Token::TypeTag::KEYWORD) {
    switch (peek().type.keywordToken) {
      case KeywordToken::CONSTANT:
      case KeywordToken::VARIABLE:
        return parseVarDeclaration();
      case KeywordToken::FUNCTION:
        return parseFunctionDeclaration();
      case KeywordToken::IF:
        return parseConditionalStatement();
      case KeywordToken::WHILE:
        return parseWhileStatement();
      case KeywordToken::DO:
        return parseDoWhileStatement();
      case KeywordToken::FOR:
        return parseForStatement();
      case KeywordToken::RETURN:
        return parseReturnStatement();
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

std::vector<std::unique_ptr<Stmt>> Parser::parseStatementBlock() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    statements.push_back(parseStatement());
    skipSemicolon();
  }
  assertToken("SyntaxToken::CLOSE_BRACE", "Expected '}' to close statement block");
  return statements;
}

std::unique_ptr<Stmt> Parser::parseFunctionDeclaration() {
  advance();
  std::string name = assertToken("DataToken::IDENTIFIER", "Expected function name following fn keyword").value;

  auto args = parseArgs();
  std::vector<std::string> params;

  for (const auto& arg : args) {
    if (arg->kind != NodeType::Identifier) {
      std::cerr << arg.get() << std::endl;
      throw std::invalid_argument("Expected identifiers in function arguments list.");
    }
    params.push_back(static_cast<Identifier*>(arg.get())->symbol);
  }

  assertToken("SyntaxToken::OPEN_BRACE", "Expected function body following declaration");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();

  auto fn = std::make_unique<FunctionDeclaration>(name, std::move(params), std::move(body));

  return fn;
}

std::unique_ptr<Stmt> Parser::parseVarDeclaration() {
  bool isConstant = (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::CONSTANT);
  advance();
  std::string identifier =
      assertToken("DataToken::IDENTIFIER", "Expected identifier name following var | const keywords.").value;

  std::unique_ptr<Expr> value = nullptr;
  if (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::ASSIGN) {
    assertToken("OperatorToken::ASSIGN", "Expected assign token following identifier in var declaration.");
    value = parseExpression();
  }
  return std::make_unique<VarDeclaration>(isConstant, identifier, std::move(value));
}

std::unique_ptr<Stmt> Parser::parseConditionalStatement() {
  // if clause
  assertToken("KeywordToken::IF", "Expected 'if' keyword to start conditional statement.");
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected open parenthesis following 'if' keyword.");
  auto condition = parseLogicalExpr();
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected closing parenthesis following if condition.");
  assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after if condition.");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
  std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>> ifClause = {std::move(condition),
                                                                                   std::move(body)};

  // elif clauses
  std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt>>>> elifClauses = {};
  while (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::ELSE_IF) {
    assertToken("KeywordToken::ELSE_IF", "Expected 'elif' keyword to start elif clause.");
    assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'elif'.");
    auto elifCondition = parseLogicalExpr();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after elif condition.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after elif condition.");
    auto elifBody = parseStatementBlock();
    elifClauses.push_back({std::move(elifCondition), std::move(elifBody)});
  }

  // else clause
  std::vector<std::unique_ptr<Stmt>> elseBody = {};
  if (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::ELSE) {
    assertToken("KeywordToken::ELSE", "Expected 'else' keyword to start else clause.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after 'else'.");
    elseBody = parseStatementBlock();
  }

  return std::make_unique<ConditionalStatement>(std::move(ifClause), std::move(elifClauses), std::move(elseBody));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
  advance();
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'while' keyword.");
  auto condition = parseLogicalExpr();
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after while condition.");
  assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after while condition.");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseDoWhileStatement() {
  advance();
  assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after 'do' keyword.");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
  assertToken("KeywordToken::WHILE", "Expected 'while' keyword after do-while body.");
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'while' keyword.");
  auto condition = parseLogicalExpr();
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after while condition.");
  assertToken("SyntaxToken::SEMICOLON", "Expected ';' after do-while statement.");
  return std::make_unique<DoWhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
  advance();
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'for' keyword.");
  std::unique_ptr<Stmt> iterator = parseVarDeclaration();
  // Handle 'for-in' loop
  if (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::IN) {
    advance();
    std::unique_ptr<Expr> iterable = parseExpression();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after 'in' expression.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after for-in loop.");
    std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
    return std::make_unique<ForInStmt>(std::move(iterator), std::move(iterable), std::move(body));
  }
  // Handle 'for-of' loop
  else if (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::OF) {
    advance();
    std::unique_ptr<Expr> iterable = parseExpression();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after 'of' expression.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after for-of loop.");
    std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
    return std::make_unique<ForOfStmt>(std::move(iterator), std::move(iterable), std::move(body));
  }
  // Handle basic for loop
  else {
    assertToken("SyntaxToken::SEMICOLON", "Expected ';' after for loop iterator.");
    std::unique_ptr<Expr> condition = parseExpression();
    assertToken("SyntaxToken::SEMICOLON", "Expected ';' after for loop condition.");
    std::unique_ptr<Expr> increment = parseExpression();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after for loop increment statement.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after for loop increment statement.");
    std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
    return std::make_unique<ForStmt>(std::move(iterator), std::move(condition), std::move(increment), std::move(body));
  }
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
  assertToken("KeywordToken::RETURN", "Expected 'return' keyword to start return statement.");
  std::unique_ptr<Expr> value = nullptr;
  if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::SEMICOLON)) {
    value = parseExpression();
  }
  assertToken("SyntaxToken::SEMICOLON", "Expected ';' after return statement.");
  return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Expr> Parser::parseExpression() { return parseAssignmentExpr(); }

std::unique_ptr<Expr> Parser::parseAssignmentExpr() {
  auto left = parseLogicalExpr();

  if (peek().tag == Token::TypeTag::OPERATOR) {
    std::unique_ptr<Expr> value = nullptr;
    std::string op;
    std::unique_ptr<Expr> expression = nullptr;
    switch (peek().type.operatorToken) {
      case OperatorToken::ASSIGN:
        advance();
        value = parseAssignmentExpr();
        expression = std::make_unique<AssignmentExpr>(std::move(left), std::move(value));
        assertToken("SyntaxToken::SEMICOLON", "Expected ';' after assignment expression.");
        return expression;
      case OperatorToken::ADDITION_ASSIGNMENT:
      case OperatorToken::SUBTRACTION_ASSIGNMENT:
      case OperatorToken::MULTIPLICATION_ASSIGNMENT:
      case OperatorToken::DIVISION_ASSIGNMENT:
      case OperatorToken::MODULUS_ASSIGNMENT:
        op = advance().value;
        value = parseAssignmentExpr();
        expression = std::make_unique<CompoundAssignmentExpr>(std::move(left), std::move(value), op);
        assertToken("SyntaxToken::SEMICOLON", "Expected ';' after assignment expression.");
        return expression;
      default:
        break;
    }
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseLogicalExpr() {
  auto left = parseEqualityExpr();

  while (peek().value == "&&" || peek().value == "||") {
    std::string op = advance().value;
    auto right = parseEqualityExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseEqualityExpr() {
  auto left = parseRelationalExpr();

  while (peek().value == "==" || peek().value == "!=") {
    std::string op = advance().value;
    auto right = parseRelationalExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseRelationalExpr() {
  auto left = parseObjectExpr();

  while (peek().value == "<" || peek().value == ">" || peek().value == "<=" || peek().value == ">=") {
    std::string op = advance().value;
    auto right = parseObjectExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), op);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseObjectExpr() {
  if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACE)) {
    return parseArrayExpr();
  }

  advance();
  std::vector<std::unique_ptr<Property>> properties;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    std::string key = assertToken("DataToken::IDENTIFIER", "Object literal key expected").value;

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

std::unique_ptr<Expr> Parser::parseArrayExpr() {
  if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACKET)) {
    return parseAdditiveExpr();
  }

  advance();
  std::vector<std::unique_ptr<Expr>> elements;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACKET)) {
    elements.push_back(parseExpression());
    if (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACKET)) {
      assertToken("SyntaxToken::COMMA", "Expected comma or closing bracket following array element");
    }
  }

  assertToken("SyntaxToken::CLOSE_BRACKET", "Array literal missing closing bracket.");
  return std::make_unique<ArrayLiteral>(std::move(elements));
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
  auto expression = parseMemberExpr();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    return parseCallExpr(std::move(expression));
  }

  if (peek().tag == Token::TypeTag::OPERATOR && (peek().type.operatorToken == OperatorToken::INCREMENT ||
                                                 peek().type.operatorToken == OperatorToken::DECREMENT)) {
    Token operatorToken = advance();
    return std::make_unique<UnaryExpr>(operatorToken, std::move(expression), false);
  }

  return expression;
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
        throw std::invalid_argument("Cannot use dot operator without right-hand side being an identifier.");
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
      case DataToken::BOOLEAN_LITERAL:
        return std::make_unique<BooleanLiteral>(BooleanLiteral{advance().value == "true"});
      case DataToken::NULL_LITERAL:
        return std::make_unique<NullLiteral>(NullLiteral{});
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
      case SyntaxToken::SINGLE_QUOTE:
        advance();
        value = assertToken("DataToken::CHAR_LITERAL", "Expected character literal").value;
        assertToken("SyntaxToken::SINGLE_QUOTE", "Expected closing single quote");
        return std::make_unique<CharLiteral>(CharLiteral{value[0]});
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
  } else if (token.tag == Token::TypeTag::OPERATOR) {
    switch (token.type.operatorToken) {
      case OperatorToken::INCREMENT:
      case OperatorToken::DECREMENT:
        return std::make_unique<UnaryExpr>(advance(), parsePrimaryExpr(), true);
      default:
        break;
    }
  }
  throw std::logic_error("Unexpected token \"" + token.plainText + "\" found in primary expression.");
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
    throw std::invalid_argument("Unexpected token found: " + token.plainText + "\n" + errorMessage);
  }
  return token;
}

void Parser::log(const std::unique_ptr<Program>& program, bool devMode) {
  if (devMode) {
    std::cout << program->toString() << std::endl;
  }
}

void Parser::skipSemicolon() {
  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::SEMICOLON) {
    advance();
  }
}