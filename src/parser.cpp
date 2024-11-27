#include "parser.h"
#include "ast_node.h"
#include "create_stmt.h"
#include "object_database.h"
#include <memory>

Parser::Parser() {
  this->ptr = 0;
}

size_t Parser::eat() {
  return this->ptr++;
}


size_t Parser::consume(const std::vector<Token>& tokens, Token::TokenType type) {
  if (tokens[this->ptr].type == type) {
    this->ptr += 1;
    return 0;
  }
  return 1;
}

OptionalNode Parser::error(const Token& token) {
  std::cerr << "[ERROR] " << "Line " << token.line << ": Invalid token " << token.value << std::endl;
  OptionalNode returnVal(nullptr);
  returnVal.error = 1;
  return returnVal;
}

Optional<std::unique_ptr<ObjectDatabase>> Parser::objectDatabase(const std::vector<Token>& tokens) {
  // Consumes TOKEN_DATABASE
  if (this->consume(tokens, Token::TOKEN_DATABASE)!= 0) {
    std::cerr << "[ERROR] " << "Line " << tokens[this->ptr].line << ": Invalid token " 
              << tokens[this->ptr].value << std::endl;
    Optional<std::unique_ptr<ObjectDatabase>> returnVal(nullptr);
    returnVal.error = 1;
    return returnVal;
  }
  std::unique_ptr<ObjectDatabase> database(new ObjectDatabase(tokens[this->eat()].value));
  return Optional<std::unique_ptr<ObjectDatabase>>(std::move(database));
}

OptionalNode Parser::createStmt(const std::vector<Token>& tokens) {
  switch (tokens[this->ptr].type) {
    case (Token::TOKEN_DATABASE): {
      Optional<std::unique_ptr<ObjectDatabase>> databaseOrError = this->objectDatabase(tokens);
      if (databaseOrError.error != 0) {
        return OptionalNode(nullptr);
      }
      std::unique_ptr<Object> database = databaseOrError.unwrap();
      std::unique_ptr<ASTNode> result(new CreateStmt(std::move(database)));
      return OptionalNode(std::move(result));
    }
    default:
      return this->error(tokens[this->ptr]);
  }
}

OptionalNode Parser::stmt(const std::vector<Token>& tokens) {
  switch (tokens[this->ptr].type) {
    case (Token::TOKEN_CREATE): {
      this->eat();
      return this->createStmt(tokens);
    }
    default:
      std::cerr << "[ERROR] Invalid token: " << tokens[this->ptr].value << std::endl;
      OptionalNode returnVal(nullptr);
      returnVal.error = 1;
      return returnVal;
  }
}


Optional<AST> Parser::parse(const std::vector<Token>& tokens) {
  this->ptr = 0;
  AST ast;
  Optional<AST> result = Optional<AST>();
  result.error = 1;

  while (this->ptr < tokens.size()) {
    if (tokens[this->ptr].type == Token::TOKEN_SEMICOLON) {
      this->ptr++;
      continue;
    }
    OptionalNode nodeOrError = this->stmt(tokens);
    if (nodeOrError.error != 0) {
      return result;
    }
    std::unique_ptr<ASTNode> node = nodeOrError.unwrap();

    ast.addRoot(std::move(node));

    if (this->ptr < tokens.size() and tokens[this->ptr].type != Token::TOKEN_SEMICOLON) {
      this->error(tokens[this->ptr]);
      return result;
    }
  }

  result.setValue(std::move(ast));
  result.error = 0;
  return result;
}
