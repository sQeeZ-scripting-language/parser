# sQeeZ Parser
The sQeeZ Parser is responsible for analyzing the sequence of tokens generated by the sQeeZ Lexer and organizing them into a structured representation known as an Abstract Syntax Tree (AST). This structured representation allows the interpreter to process and execute the sQeeZ script. The parser follows the core principles of sQeeZ: compactness, clarity through convention, and an emphasis on efficient handling of data structures. It handles various expressions, control structures, and function declarations to create a seamless and flexible programming experience.

# Table of Contents
- [How to Use](#how-to-use)
  - [Integrate the Lexer Library](#integrate-the-lexer-library)
  - [Build & Run](#build--run)
  - [Testing](#testing)
  - [Code Formatting](#code-formatting)
- [Abstract Syntax Tree (AST)](#abstract-syntax-tree-ast)
  - [Basic AST Node](#basic-ast-node)
  - [Statements](#statements)
  - [Expressions](#expressions)
  - [Overview of the Node Types](#overview-of-the-node-types)
- [Order of Precedence](#order-of-precedence)
  - [Operator Precedence Levels](#operator-precedence-levels)
  - [Parsing Functions Overview](#parsing-functions-overview)

# How to Use
> **Note:**
> 
> For convenience, each category (Lexer, Build, Testing, Code Formatting) has a corresponding script:
> 
> - `latest_lexer_release.sh`
> - `build.sh`
> - `test.sh`
> - `checkstyle.sh`
> 
> These scripts can be run directly from the root directory of the project to automate the respective tasks.

## Integrate the Lexer Library
To integrate the latest release of the sQeeZ Lexer, follow these steps:

### 1. Integrate latest Lexer Release
```bash
#!/bin/bash

REPO="sQeeZ-scripting-language/lexer"
REPO_DIR="./lexer-lib"
INCLUDE_DIR="./include/lexer"

if [ -d "$INCLUDE_DIR" ]; then
  rm -rf "$INCLUDE_DIR"
fi
mkdir -p "$INCLUDE_DIR"
mkdir -p "$REPO_DIR"

LATEST_RELEASE=$(curl -s "https://api.github.com/repos/$REPO/releases/latest")

if [[ "$OSTYPE" == "darwin"* ]]; then
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-macos-.*\.zip" | head -n 1)
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-linux-.*\.zip" | head -n 1)
elif [[ "$OSTYPE" == "msys"* ]]; then
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-windows-.*\.zip" | head -n 1)
else
  echo "Unsupported OS."
  exit 1
fi

if [ -z "$ZIP_URL" ]; then
  echo "No matching asset found."
  exit 1
fi

TEMP_DIR=$(mktemp -d)

curl -L -o "$TEMP_DIR/sQeeZ-Lexer.zip" "$ZIP_URL"

unzip "$TEMP_DIR/sQeeZ-Lexer.zip" -d "$REPO_DIR"

mv "$REPO_DIR/"*/libsQeeZ-Lexer-Lib.a "$REPO_DIR/"
mv "$REPO_DIR/"*/sQeeZ-Lexer-Lib.lib "$REPO_DIR/"
mv "$REPO_DIR/lexer/"* "$INCLUDE_DIR/"

rm -rf "$REPO_DIR/"*/sQeeZ-Lexer-Exe
rm -rf "$REPO_DIR/"*/sQeeZ-Lexer-Exe.exe
rm -rf "$REPO_DIR/"*/sQeeZ-Lexer-Node.node
rm -rf "$INCLUDE_DIR/node"
rm -rf "$TEMP_DIR"

find "$REPO_DIR" -type d -empty -delete

echo "Latest files have been downloaded and extracted to $REPO_DIR."
```

## Build & Run
To compile the project, follow these steps:

### 1. Create and Navigate to Build Directory
```bash
mkdir build
cd build
```

### 2. Configure the Project with CMake
```bash
cmake ..
```

### 3. Build the Project
```bash
cmake --build .
```

### 4. Run the generated Executable
```bash
cd build
./sQeeZ-Parser-Exe $FILE_PATH.sqz [--flag]
```

## Testing
To run the tests, do the following:

### 1. Navigate to the Build Directory
```bash
cd build
```

### 2. Execute the Tests
```bash
./parser_test
```
or
```bash
ctest --output-on-failure
```

## Code Formatting
To format your code, execute the following script, which will apply the clang-format to all .cpp and .hpp files in the project directory:

### 1. Apply clang-format
```bash
function format_file {
  local file=$1
  clang-format -i "$file"
}

for file in $(find . -name '*.cpp' -o -name '*.hpp'); do
  format_file "$file"
done
```

# Abstract Syntax Tree (AST)
An abstract syntax tree (AST) is a data structure used in computer science to represent the structure of a program or code snippet. It is a tree representation of the abstract syntactic structure of text written in a formal language.

## Basic AST Node
The ASTNode class is the abstract base class for all nodes in the Abstract Syntax Tree (AST). It serves as the foundation for every specific type of node (such as expressions and statements) within the AST. The ASTNode class defines the basic interface that all derived nodes must follow, ensuring that they can be represented in a uniform way.

```cpp
class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual std::string toString() const = 0;
};
```

## Statements
In the Abstract Syntax Tree (AST), statements represent the actions or instructions that are executed in the program, but they do not result in a runtime value.

```cpp
class Stmt : public ASTNode {
public:
  NodeType kind;
  explicit Stmt(const NodeType& kind) : kind(kind) {}
};
```

## Expressions
In the Abstract Syntax Tree (AST), expressions are constructs that evaluate to produce a runtime value. Unlike statements, which primarily control the flow of execution, expressions are used to compute values that can be utilized by other parts of the program.

```cpp
class Expr : public Stmt {
public:
  explicit Expr(const NodeType& kind) : Stmt(kind) {}
};
```

## Overview of the Node Types
The NodeType enum class defines all the different types of nodes that can appear in the Abstract Syntax Tree (AST). Each type corresponds to a specific syntactic structure in the source code, categorized into Statements, Expressions, and Literals. Notably, literals are treated as the most basic form of expressions within the AST.

```cpp
enum class NodeType {
  // STATEMENTS
  Program,
  FunctionDeclaration,
  ReturnStmt,
  VarDeclaration,
  ConditionalStmt,
  WhileStmt,
  DoWhileStmt,
  ForStmt,
  ForInStmt,
  ForOfStmt,
  LogStmt,
  // EXPRESSIONS
  AssignmentExpr,
  CompoundAssignmentExpr,
  CallbackFunctionExpr,
  TernaryExpr,
  BinaryExpr,
  UnaryExpr,
  CallExpr,
  MemberExpr,
  // LITERALS
  Property,
  ObjectLiteral,
  ArrayLiteral,
  Identifier,
  NullLiteral,
  IntegerLiteral,
  DoubleLiteral,
  BooleanLiteral,
  CharLiteral,
  StringLiteral,
  HexCodeLiteral,
  // Short Notation
  ShortOperationExpr
};
```

# Order of Precedence
The order of precedence dictates how operators are parsed and evaluated in expressions, which is vital for constructing accurate Abstract Syntax Trees (AST). The parser employs recursive descent parsing, where each level of the tree represents different precedence levels. This recursive nature allows for a clear and structured representation of nested expressions, ensuring that operations are performed in the correct order according to their precedence.

## Operator Precedence Levels

1. **Primary Expressions**  
   - Literals, Identifiers, Parenthesized expressions (`parsePrimaryExpr`)

2. **Short Notation (Data)**  
   - `@` (short notation for objects and arrays) (`parseShortData`)

3. **Short Notation (Functions)**  
   - `MAP`, `FILTER`, `REDUCE`, etc. (`parseCallbackFunctionExpr`)

4. **Member Access**  
   - `.` (dot notation), `[]` (bracket notation), `|>` (pipe notation) (`parseMemberExpr`)

5. **Function Calls**  
   - `()` (function call) (`parseCallExpr`, `parseCallMemberExpr`)

6. **Unary Operators**  
   - `++`, `--` (pre-increment and pre-decrement)

7. **Exponentiation**  
   - `**` (power operator) (`parsePowerExpr`)

8. **Multiplicative Operators**  
   - `*`, `/`, `%` (multiplication, division, modulus) (`parseMultiplicativeExpr`)

9. **Additive Operators**  
   - `+`, `-` (addition and subtraction) (`parseAdditiveExpr`)

10. **Relational Operators**  
    - `<`, `<=`, `>`, `>=` (comparison) (`parseRelationalExpr`)

11. **Equality Operators**  
    - `==`, `!=` (equality checks) (`parseEqualityExpr`)

12. **Logical Operators**  
    - `&&`, `||` (logical AND and OR) (`parseLogicalExpr`)

13. **Conditional Operator**  
    - `? :` (ternary operator) (`parseTernaryExpr`)

14. **Assignment Operators**  
    - `=`, `+=`, `-=`, `*=`, `/=`, `%=`, etc. (`parseAssignmentExpr`)

## Parsing Functions Overview

### Statements
- `parseStatement()`
- `parseStatementBlock()`
- `parseFunctionDeclaration()`
- `parseReturnStatement()`
- `parseVarDeclaration()`
- `parseConditionalStatement()`
- `parseWhileStatement()`
- `parseDoWhileStatement()`
- `parseForStatement()`
- `parseLogStatement()`

### Expressions
- `parseExpression()`
- `parseAssignmentExpr()`
- `parseTernaryExpr()`
- `parseLogicalExpr()`
- `parseEqualityExpr()`
- `parseRelationalExpr()`
- `parseObjectExpr()`
- `parseArrayExpr()`
- `parseCallbackFunctionExpr()`
- `parseShortData()`
- `parseAdditiveExpr()`
- `parseMultiplicativeExpr()`
- `parsePowerExpr()`
- `parseCallMemberExpr()`
- `parseCallExpr(caller, method)`
- `parseShortExpr(caller, method)`
- `parseArgs()`
- `parseShortArgs()`
- `parseArgumentsList()`
- `parseMemberExpr()`
- `parsePrimaryExpr()`


[Back to Top](#sqeez-parser)