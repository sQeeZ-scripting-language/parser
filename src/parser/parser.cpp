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

  std::cerr << "\033[1;30m\033[41m[sQeeZ]: Exception of type: " << exceptionType << " - Message: " << e.what()
            << "\033[0m" << std::endl;
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
    skipComment();
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
  assertToken("KeywordToken::FUNCTION", "Expected 'fn' keyword to start function declaration.");
  Token name = assertToken("DataToken::IDENTIFIER", "Expected function name following fn keyword");

  auto args = parseArgs();
  std::vector<Token> params;

  for (const auto& arg : args) {
    if (arg->kind != NodeType::Identifier) {
      std::cerr << arg.get() << std::endl;
      throw std::invalid_argument("Expected identifiers in function arguments list.");
    }
    params.push_back(static_cast<Identifier*>(arg.get())->identifier);
  }

  assertToken("SyntaxToken::OPEN_BRACE", "Expected function body following declaration");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();

  auto fn = std::make_unique<FunctionDeclaration>(name, std::move(params), std::move(body));

  return fn;
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

std::unique_ptr<Stmt> Parser::parseVarDeclaration() {
  Token type = advance();  // var | const
  bool declarationDone = false;
  std::vector<std::pair<Token, std::unique_ptr<ASTNode>>> declarations = {};
  std::unique_ptr<Expr> value = nullptr;

  while (!declarationDone) {
    Token identifier = assertToken("DataToken::IDENTIFIER", "Expected identifier name following var | const keywords.");

    if (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::ASSIGN) {
      assertToken("OperatorToken::ASSIGN", "Expected assign token following identifier in var declaration.");
      value = parseExpression();
    } else {
      value = nullptr;
    }

    declarations.push_back({identifier, value ? std::move(value) : nullptr});

    if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
      assertToken("SyntaxToken::COMMA", "Expected comma for chaining multiple declarations.");
    } else {
      declarationDone = true;
    }
  }

  return std::make_unique<VarDeclaration>(type, std::move(declarations));
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

  return std::make_unique<ConditionalStmt>(std::move(ifClause), std::move(elifClauses), std::move(elseBody));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
  assertToken("KeywordToken::WHILE", "Expected 'while' keyword to start while statement.");
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'while' keyword.");
  auto condition = parseLogicalExpr();
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after while condition.");
  assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after while condition.");
  std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseDoWhileStatement() {
  assertToken("KeywordToken::DO", "Expected 'do' keyword to start do-while statement.");
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
  assertToken("KeywordToken::FOR", "Expected 'for' keyword to start for statement.");
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after 'for' keyword.");
  std::unique_ptr<Stmt> iterator = parseVarDeclaration();
  // Handle 'for-in' loop
  if (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::IN) {
    assertToken("KeywordToken::IN", "Expected 'in' keyword after for loop iterator.");
    std::unique_ptr<Expr> iterable = parseExpression();
    assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after 'in' expression.");
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' after for-in loop.");
    std::vector<std::unique_ptr<Stmt>> body = parseStatementBlock();
    return std::make_unique<ForInStmt>(std::move(iterator), std::move(iterable), std::move(body));
  }
  // Handle 'for-of' loop
  else if (peek().tag == Token::TypeTag::KEYWORD && peek().type.keywordToken == KeywordToken::OF) {
    assertToken("KeywordToken::OF", "Expected 'of' keyword after for loop iterator.");
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

std::unique_ptr<Stmt> Parser::parseLogStatement() {
  Token logType = advance();  // log | logc | warn | error
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' after log function call.");
  std::vector<std::unique_ptr<Expr>> message = {};
  do {
    if (message.size() > 0) {
      assertToken("SyntaxToken::COMMA", "Expected ',' between message expressions in log function call.");
    }
    message.push_back(parseExpression());
  } while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA);
  std::unique_ptr<Expr> colorExpr = nullptr;
  if (logType.tag == Token::TypeTag::LOG && logType.type.logToken == LogToken::COLORED) {
    colorExpr = std::move(message.back());
    message.pop_back();
    if (colorExpr->kind != NodeType::HexCodeLiteral) {
      throw std::invalid_argument("Expected hex code literal as last argument for a colored log.");
    }
  }
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected ')' after log function call.");
  skipSemicolon();
  return std::make_unique<LogStmt>(logType, std::move(message), std::move(colorExpr));
}

std::unique_ptr<Expr> Parser::parseExpression() { return parseAssignmentExpr(); }

std::unique_ptr<Expr> Parser::parseAssignmentExpr() {
  auto left = parseTernaryExpr();

  if (peek().tag == Token::TypeTag::OPERATOR) {
    std::unique_ptr<Expr> value = nullptr;
    Token operator_ = peek();
    std::unique_ptr<Expr> expression = nullptr;
    switch (peek().type.operatorToken) {
      case OperatorToken::ASSIGN:
        assertToken("OperatorToken::ASSIGN", "Expected '=' after assignment expression.");
        value = parseAssignmentExpr();
        expression = std::make_unique<AssignmentExpr>(std::move(left), std::move(value));
        skipSemicolon();
        return expression;
      case OperatorToken::ADDITION_ASSIGNMENT:
      case OperatorToken::SUBTRACTION_ASSIGNMENT:
      case OperatorToken::MULTIPLICATION_ASSIGNMENT:
      case OperatorToken::DIVISION_ASSIGNMENT:
      case OperatorToken::MODULUS_ASSIGNMENT:
        operator_ = advance();  // + | - | * | / | %
        value = parseAssignmentExpr();
        expression = std::make_unique<CompoundAssignmentExpr>(std::move(left), std::move(value), operator_);
        skipSemicolon();
        return expression;
      default:
        break;
    }
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseTernaryExpr() {
  auto condition = parseLogicalExpr();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::QUESTION_MARK) {
    assertToken("SyntaxToken::QUESTION_MARK", "Expected '?' after condition in ternary operator.");
    auto trueExpr = parseExpression();
    assertToken("SyntaxToken::COLON", "Expected ':' after true expression in ternary operator.");
    auto falseExpr = parseExpression();
    return std::make_unique<TernaryExpr>(std::move(condition), std::move(trueExpr), std::move(falseExpr));
  }

  return condition;
}

std::unique_ptr<Expr> Parser::parseLogicalExpr() {
  auto left = parseEqualityExpr();
  while (peek().tag == Token::TypeTag::LOGICAL &&
         (peek().type.logicalToken == LogicalToken::AND || peek().type.logicalToken == LogicalToken::OR)) {
    Token operator_ = advance();  // && | ||
    auto right = parseEqualityExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseEqualityExpr() {
  auto left = parseRelationalExpr();

  while (peek().tag == Token::TypeTag::LOGICAL &&
         (peek().type.logicalToken == LogicalToken::EQUAL || peek().type.logicalToken == LogicalToken::NOT_EQUAL)) {
    Token operator_ = advance();  // == | !=
    auto right = parseRelationalExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseRelationalExpr() {
  auto left = parseAdditiveExpr();

  while (peek().tag == Token::TypeTag::LOGICAL &&
         (peek().type.logicalToken == LogicalToken::LESS || peek().type.logicalToken == LogicalToken::GREATER ||
          peek().type.logicalToken == LogicalToken::LESS_EQUAL ||
          peek().type.logicalToken == LogicalToken::GREATER_EQUAL)) {
    Token operator_ = advance();  // < | > | <= | >=
    auto right = parseAdditiveExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseObjectExpr() {
  assertToken("SyntaxToken::OPEN_BRACE", "Expected opening brace for object");
  std::vector<std::unique_ptr<Property>> properties;

  while (!isEOF() && !(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_BRACE)) {
    Token key = assertToken("DataToken::IDENTIFIER", "Object literal key expected");

    // pair -> { key, }
    if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
      assertToken("SyntaxToken::COMMA", "Expected value chain following comma in ObjectExpr");
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
  assertToken("SyntaxToken::OPEN_BRACKET", "Expected opening bracket for array");
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

std::unique_ptr<Expr> Parser::parseCallbackFunctionExpr() {
  std::vector<Token> params;
  while (!(peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::CLOSE_PARENTHESIS)) {
    if (params.size() > 0) {
      assertToken("SyntaxToken::COMMA", "Expected comma between callback function parameters.");
    }
    params.push_back(assertToken("DataToken::IDENTIFIER", "Expected identifier in callback function parameters."));
  }
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Expected closing parenthesis after callback function parameters.");
  assertToken("SyntaxToken::CALLBACK", "Expected '=>' to start callback function body.");
  std::vector<std::unique_ptr<Stmt>> body = {};
  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACE) {
    assertToken("SyntaxToken::OPEN_BRACE", "Expected '{' to start callback function body.");
    body = parseStatementBlock();
  } else {
    body.push_back(parseStatement());
    skipSemicolon();
  }
  skipSemicolon();
  return std::make_unique<CallbackFunctionExpr>(std::move(params), std::move(body));
}

std::unique_ptr<Expr> Parser::parseShortData() {
  assertToken("SyntaxToken::AT", "Expected '@' to start short data notation.");
  // Short Notation -> Object @ key:value, key:value
  if (lookAhead(2).tag == Token::TypeTag::SYNTAX && lookAhead(2).type.syntaxToken == SyntaxToken::COLON) {
    std::vector<std::unique_ptr<Property>> properties;
    do {
      if (properties.size() > 0) {
        assertToken("SyntaxToken::COMMA", "Expected comma between properties in short data notation.");
      }
      Token key = assertToken("DataToken::IDENTIFIER", "Expected identifier key in short data notation.");
      assertToken("SyntaxToken::COLON", "Expected colon after key in short data notation.");
      properties.push_back(std::make_unique<Property>(Property{key, std::move(parseExpression())}));
    } while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA);
    assertToken("SyntaxToken::SEMICOLON", "Expected semicolon after short data notation.");
    return std::make_unique<ObjectLiteral>(std::move(properties));
  }
  // Short Notation -> Array @ value, value, value
  else {
    std::vector<std::unique_ptr<Expr>> elements;
    do {
      if (elements.size() > 0) {
        assertToken("SyntaxToken::COMMA", "Expected comma between elements in short data notation.");
      }
      elements.push_back(parseExpression());
    } while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA);
    assertToken("SyntaxToken::SEMICOLON", "Expected semicolon after short data notation.");
    return std::make_unique<ArrayLiteral>(std::move(elements));
  }
}

std::unique_ptr<Expr> Parser::parseAdditiveExpr() {
  auto left = parseMultiplicativeExpr();

  while (peek().tag == Token::TypeTag::OPERATOR && (peek().type.operatorToken == OperatorToken::ADDITION ||
                                                    peek().type.operatorToken == OperatorToken::SUBTRACTION)) {
    Token operator_ = advance();  // + | -
    auto right = parseMultiplicativeExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicativeExpr() {
  auto left = parsePowerExpr();

  while (peek().tag == Token::TypeTag::OPERATOR && (peek().type.operatorToken == OperatorToken::MULTIPLICATION ||
                                                    peek().type.operatorToken == OperatorToken::DIVISION ||
                                                    peek().type.operatorToken == OperatorToken::MODULUS)) {
    Token operator_ = advance();  // * | / | %
    auto right = parsePowerExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parsePowerExpr() {
  auto left = parseCallMemberExpr();

  while (peek().tag == Token::TypeTag::OPERATOR && peek().type.operatorToken == OperatorToken::POTENTIATION) {
    Token operator_ = assertToken("OperatorToken::POTENTIATION", "Expected '**' for power operator");
    auto right = parseCallMemberExpr();
    left = std::make_unique<BinaryExpr>(std::move(left), std::move(right), operator_);
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseCallMemberExpr() {
  auto expression = parseMemberExpr();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
    return parseCallExpr(nullptr, std::move(expression));
  }

  if (peek().tag == Token::TypeTag::OPERATOR && (peek().type.operatorToken == OperatorToken::INCREMENT ||
                                                 peek().type.operatorToken == OperatorToken::DECREMENT)) {
    Token operatorToken = advance();  // ++ | --
    return std::make_unique<UnaryExpr>(operatorToken, std::move(expression), false);
  }

  return expression;
}

std::unique_ptr<Expr> Parser::parseCallExpr(std::unique_ptr<Expr> caller, std::unique_ptr<Expr> method) {
  auto callExpr = std::make_unique<CallExpr>();
  callExpr->caller = std::move(caller);
  callExpr->method = std::move(method);
  callExpr->args = parseArgs();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::DOT) {
    assertToken("SyntaxToken::DOT", "Expected dot operator following method call");
    auto followingMethod = parsePrimaryExpr();
    callExpr = std::unique_ptr<CallExpr>(
        static_cast<CallExpr*>(parseCallExpr(std::move(callExpr), std::move(followingMethod)).release()));
  } else if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::PIPE_OPERATOR) {
    assertToken("SyntaxToken::PIPE_OPERATOR", "Expected pipe operator following method call");
    auto followingMethod = parsePrimaryExpr();
    callExpr = std::unique_ptr<CallExpr>(
        static_cast<CallExpr*>(parseShortExpr(std::move(callExpr), std::move(followingMethod)).release()));
  }

  return callExpr;
}

std::unique_ptr<Expr> Parser::parseShortExpr(std::unique_ptr<Expr> caller, std::unique_ptr<Expr> method) {
  auto callExpr = std::make_unique<CallExpr>();
  callExpr->caller = std::move(caller);
  callExpr->method = std::move(method);
  callExpr->args = parseShortArgs();

  if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::DOT) {
    assertToken("SyntaxToken::DOT", "Expected dot operator following method call");
    auto followingMethod = parsePrimaryExpr();
    callExpr = std::unique_ptr<CallExpr>(
        static_cast<CallExpr*>(parseCallExpr(std::move(callExpr), std::move(followingMethod)).release()));
  } else if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::PIPE_OPERATOR) {
    assertToken("SyntaxToken::PIPE_OPERATOR", "Expected pipe operator following method call");
    auto followingMethod = parsePrimaryExpr();
    callExpr = std::unique_ptr<CallExpr>(
        static_cast<CallExpr*>(parseShortExpr(std::move(callExpr), std::move(followingMethod)).release()));
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

std::vector<std::unique_ptr<Expr>> Parser::parseShortArgs() {
  assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected open parenthesis");
  std::vector<std::unique_ptr<Expr>> args;

  while (peek().tag != Token::TypeTag::SYNTAX && peek().type.syntaxToken != SyntaxToken::CLOSE_PARENTHESIS) {
    if (args.size() > 0) {
      assertToken("SyntaxToken::COMMA", "Expected comma between arguments");
    }
    if ((peek().tag == Token::TypeTag::LOGICAL &&
         !(peek().type.logicalToken == LogicalToken::AND || peek().type.logicalToken == LogicalToken::OR)) ||
        (peek().tag == Token::TypeTag::OPERATOR &&
         !(peek().type.operatorToken == OperatorToken::ASSIGN ||
           peek().type.operatorToken == OperatorToken::ADDITION_ASSIGNMENT ||
           peek().type.operatorToken == OperatorToken::SUBTRACTION_ASSIGNMENT ||
           peek().type.operatorToken == OperatorToken::MULTIPLICATION_ASSIGNMENT ||
           peek().type.operatorToken == OperatorToken::DIVISION_ASSIGNMENT ||
           peek().type.operatorToken == OperatorToken::MODULUS_ASSIGNMENT ||
           peek().type.operatorToken == OperatorToken::POTENTIATION_ASSIGNMENT))) {
      if (peek().type.operatorToken == OperatorToken::INCREMENT) {
        args.push_back(std::make_unique<ShortOperationExpr>(ShortOperationExpr{Token(OperatorToken::ADDITION, 1, advance().pos, "+", "OperatorToken::ADDITION", "Parsed Increment Operator"), std::make_unique<IntegerLiteral>(IntegerLiteral{1})}));
      } else if (peek().type.operatorToken == OperatorToken::DECREMENT) {
        args.push_back(std::make_unique<ShortOperationExpr>(ShortOperationExpr{Token(OperatorToken::SUBTRACTION, 1, advance().pos, "-", "OperatorToken::SUBTRACTION", "Parsed Decrement Operator"), std::make_unique<IntegerLiteral>(IntegerLiteral{1})}));
      } else {
        args.push_back(std::make_unique<ShortOperationExpr>(ShortOperationExpr{advance(), parseExpression()}));
      }
    } else {
      args.push_back(parseExpression());
    }
  }
  assertToken("SyntaxToken::CLOSE_PARENTHESIS", "Missing closing parenthesis inside arguments list");
  return args;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgumentsList() {
  std::vector<std::unique_ptr<Expr>> args;
  args.push_back(parseAssignmentExpr());

  while (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::COMMA) {
    assertToken("SyntaxToken::COMMA", "Expected comma between arguments");
    args.push_back(parseAssignmentExpr());
  }

  return args;
}

std::unique_ptr<Expr> Parser::parseMemberExpr() {
  std::unique_ptr<Expr> object = parsePrimaryExpr();

  while ((peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::DOT) ||
         (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_BRACKET) ||
         (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::PIPE_OPERATOR)) {
    Token token = advance();  // . | [ | |>
    std::unique_ptr<Expr> property;

    // dot notation
    if (token.type.syntaxToken == SyntaxToken::DOT) {
      property = parsePrimaryExpr();
      if (property->kind != NodeType::Identifier) {
        throw std::invalid_argument("Cannot use dot operator without right-hand side being an identifier.");
      }
      // method call
      if (peek().tag == Token::TypeTag::SYNTAX && peek().type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
        object = parseCallExpr(std::move(object), std::move(property));
      } else {
        object = std::make_unique<MemberExpr>(MemberExpr{std::move(object), std::move(property), false});
      }
    }
    // bracket notation
    else if (token.type.syntaxToken == SyntaxToken::OPEN_BRACKET) {
      property = parseExpression();
      assertToken("SyntaxToken::CLOSE_BRACKET", "Missing closing bracket in computed value.");
      object = std::make_unique<MemberExpr>(MemberExpr{std::move(object), std::move(property), true});
    }
    // pipe operator
    else {
      if (peek().tag != Token::TypeTag::SHORT_NOTATION) {
        throw std::invalid_argument("Expected Short Notation following pipe operator.");
      }
      property = parsePrimaryExpr();
      object = parseShortExpr(std::move(object), std::move(property));
    }
  }

  return object;
}

std::unique_ptr<Expr> Parser::parsePrimaryExpr() {
  std::regex boolPattern(R"(^true|false$)");
  Token token = peek();

  if (token.tag == Token::TypeTag::DATA) {
    switch (token.type.dataToken) {
      case DataToken::INTEGER_LITERAL:
        token = assertToken("DataToken::INTEGER_LITERAL", "Expected integer literal");
        return std::make_unique<IntegerLiteral>(IntegerLiteral{std::stoi(token.value)});
      case DataToken::DOUBLE_LITERAL:
        token = assertToken("DataToken::DOUBLE_LITERAL", "Expected double literal");
        return std::make_unique<DoubleLiteral>(DoubleLiteral{std::stod(token.value)});
      case DataToken::BOOLEAN_LITERAL:
        if (!std::regex_match(peek().value, boolPattern)) {
          throw std::invalid_argument("Invalid boolean literal: " + peek().value);
        }
        token = assertToken("DataToken::BOOLEAN_LITERAL", "Expected boolean literal");
        return std::make_unique<BooleanLiteral>(BooleanLiteral{token.value == "true"});
      case DataToken::NULL_LITERAL:
        assertToken("DataToken::NULL_LITERAL", "Expected null literal");
        return std::make_unique<NullLiteral>(NullLiteral{});
      case DataToken::IDENTIFIER:
        token = assertToken("DataToken::IDENTIFIER", "Expected identifier");
        return std::make_unique<Identifier>(Identifier{token});
      default:
        break;
    }
  } else if (token.tag == Token::TypeTag::SYNTAX) {
    std::unique_ptr<Expr> expression;
    std::string value;
    int i, scope = 0;
    switch (token.type.syntaxToken) {
      case SyntaxToken::OPEN_BRACKET:
        return parseArrayExpr();
      case SyntaxToken::OPEN_BRACE:
        return parseObjectExpr();
      case SyntaxToken::AT:
        return parseShortData();
      case SyntaxToken::OPEN_PARENTHESIS:
        assertToken("SyntaxToken::OPEN_PARENTHESIS", "Expected '(' to start parenthesised expression.");
        while (true) {
          i++;
          if (lookAhead(i).tag == Token::TypeTag::SYNTAX &&
              lookAhead(i).type.syntaxToken == SyntaxToken::OPEN_PARENTHESIS) {
            scope++;
          } else if (lookAhead(i).tag == Token::TypeTag::SYNTAX &&
                     lookAhead(i).type.syntaxToken == SyntaxToken::CLOSE_PARENTHESIS) {
            if (scope == 0) {
              break;
            }
            scope--;
          }
        }
        if (lookAhead(i + 1).tag == Token::TypeTag::SYNTAX &&
            lookAhead(i + 1).type.syntaxToken == SyntaxToken::CALLBACK) {
          expression = parseCallbackFunctionExpr();
        } else {
          expression = parseExpression();
          assertToken("SyntaxToken::CLOSE_PARENTHESIS",
                      "Unexpected token found inside parenthesised expression. Expected closing parenthesis.");
        }
        return expression;
      case SyntaxToken::DOUBLE_QUOTE:
        assertToken("SyntaxToken::DOUBLE_QUOTE", "Expected opening double quote");
        value = assertToken("DataToken::STRING_LITERAL", "Expected string literal").value;
        assertToken("SyntaxToken::DOUBLE_QUOTE", "Expected closing double quote");
        return std::make_unique<StringLiteral>(StringLiteral{value});
      case SyntaxToken::SINGLE_QUOTE:
        assertToken("SyntaxToken::SINGLE_QUOTE", "Expected opening single quote");
        value = assertToken("DataToken::CHAR_LITERAL", "Expected character literal").value;
        assertToken("SyntaxToken::SINGLE_QUOTE", "Expected closing single quote");
        return std::make_unique<CharLiteral>(CharLiteral{value[0]});
      case SyntaxToken::HASHTAG:
        assertToken("SyntaxToken::HASHTAG", "Expected hashtag to start hex code literal");
        value = assertToken("DataToken::HEX_CODE_LITERAL", "Expected hex code literal").value;
        return std::make_unique<HexCodeLiteral>(HexCodeLiteral{"#" + value});
      case SyntaxToken::INLINE_COMMENT:
        skipComment();
        return nullptr;
      default:
        break;
    }
  } else if (token.tag == Token::TypeTag::OPERATOR) {
    Token nextToken = peek();
    switch (token.type.operatorToken) {
      case OperatorToken::SUBTRACTION:
        advance();  // -
        nextToken = peek();
        if (nextToken.tag == Token::TypeTag::DATA && nextToken.type.dataToken == DataToken::INTEGER_LITERAL) {
          return std::make_unique<IntegerLiteral>(IntegerLiteral{-std::stoi(advance().value)});
        } else if (nextToken.tag == Token::TypeTag::DATA && nextToken.type.dataToken == DataToken::DOUBLE_LITERAL) {
          return std::make_unique<DoubleLiteral>(DoubleLiteral{-std::stod(advance().value)});
        } else {
          throw std::invalid_argument("Unexpected token found after '-' operator: " + advance().plainText);
        }
      case OperatorToken::INCREMENT:
      case OperatorToken::DECREMENT:
        token = advance();  // ++ | --
        return std::make_unique<UnaryExpr>(token, parsePrimaryExpr(), true);
      default:
        break;
    }
  } else if (token.tag == Token::TypeTag::LOGICAL) {
    switch (token.type.logicalToken) {
      case LogicalToken::NOT:
        token = advance();  // !
        return std::make_unique<UnaryExpr>(token, parseExpression(), true);
      default:
        break;
    }
  } else if (token.tag == Token::TypeTag::SHORT_NOTATION) {
    token.value = shortEnumToString.at(advance().type.shortNotationToken);
    return std::make_unique<Identifier>(Identifier{token});
  }
  throw std::logic_error("Unexpected token \"" + token.plainText + "\" found in primary expression.");
}

// Utility functions
bool Parser::isEOF() { return peek().tag == Token::TypeTag::BASIC && peek().type.basicToken == BasicToken::TOKEN_EOF; }

Token Parser::peek(int steps) {
  if (tokens.size() < steps) {
    return tokens.back();
  } else {
    return tokens[steps - 1];
  }
}

Token Parser::lookAhead(int steps) {
  if (tokens.size() < steps) {
    throw std::runtime_error("Unexpected end of file");
  } else {
    return tokens[steps - 1];
  }
}

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
    assertToken("SyntaxToken::SEMICOLON", "Expected semicolon");
  }
}

void Parser::skipComment() {
  assertToken("SyntaxToken::INLINE_COMMENT", "Expected inline comment token");
  if (peek().tag == Token::TypeTag::DATA && peek().type.dataToken == DataToken::COMMENT_LITERAL) {
    assertToken("DataToken::COMMENT_LITERAL", "Expected comment literal");
  }
}