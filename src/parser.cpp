#include "parser.h"
#include "ast_node.h"
#include "create_stmt.h"
#include <memory>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

size_t Parser::eat() {
  return this->ptr++;
}

OptionalNode Parser::error() {
  std::cerr << "[ERROR] Line " << tokens[this->ptr].line << ": Invalid token " << tokens[this->ptr].value << std::endl;
  OptionalNode returnVal(nullptr);
  returnVal.error = 1;
  return returnVal;
}

OptionalNode Parser::createStmt() {
  switch (tokens[this->ptr].type) {
    case (Token::TOKEN_DATABASE): {
      this->eat(); // DATABASE
      if (this->tokens[this->ptr].type != Token::TOKEN_IDENTIFIER) {
        std::cerr << "[ERROR] Line " << this->tokens[this->ptr].line << ": " << "Expect identifier after DATABASE" << std::endl;
        return OptionalNode();
      }
      return std::unique_ptr<ASTNode>(new CreateStmt(CreateStmt::DATABASE, tokens[this->eat()].value));
    }
    default:
      return this->error();
  }
}

OptionalNode Parser::stmt() {
  switch (tokens[this->ptr].type) {
    case (Token::TOKEN_CREATE): {
      this->eat();
      return this->createStmt();
    }
    default:
      std::cerr << "[ERROR] Invalid token: " << tokens[this->ptr].value << std::endl;
      return OptionalNode();
  }
}


Optional<AST> Parser::parse() {
  this->ptr = 0;
  AST ast;

  while (this->ptr < tokens.size()) {
    if (tokens[this->ptr].type == Token::TOKEN_SEMICOLON) {
      this->ptr++;
      continue;
    }
    OptionalNode nodeOrError = this->stmt();
    if (nodeOrError.error != 0) {
      return Optional<AST>();
    }
    std::unique_ptr<ASTNode> node = nodeOrError.unwrap();

    ast.addRoot(std::move(node));

    if (this->ptr < tokens.size() and tokens[this->ptr].type != Token::TOKEN_SEMICOLON) {
      this->error();
      return Optional<AST>();
    }
  }

  return Optional<AST>(std::move(ast));
}
