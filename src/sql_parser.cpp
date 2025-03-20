#include "sql_parser.hpp"
#include "absl/status/status.h"
#include "ast_node.hpp"
#include "create_stmt.hpp"
#include "database_node.hpp"
#include "table_node.hpp"
#include "util.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>

SQLParser::SQLParser(const std::vector<Token> &tokens) : tokens(tokens) {}

const Token *const SQLParser::eat() { return &tokens[this->ptr++]; }

absl::StatusOr<const Token *> 
SQLParser::consume(Token::TokenType type, const std::string &message) {
  if (ptr >= tokens.size() || tokens[ptr].type != type) {
    return this->error(message);
  }
  return eat();
}

absl::StatusOr<const Token *>
SQLParser::multiConsume(const std::vector<Token::TokenType> &types,
                        const std::string &message) {
  if (ptr >= tokens.size()) {
    return absl::InvalidArgumentError("Invalid syntax");
  }
  if (std::find(types.begin(), types.end(), current()->type) == types.end()) {
    return absl::InvalidArgumentError("Invalid syntax");
  }

  return eat();
}

absl::StatusOr<const Token *> SQLParser::peek() const {
  if (this->ptr + 1 >= tokens.size()) {
    return absl::InvalidArgumentError("Invalid syntax");
  }
  return &tokens[this->ptr];
}

absl::Status SQLParser::error(const std::string &message) {
  absl::Status status =  absl::InvalidArgumentError("Line " + std::to_string(tokens[this->ptr].line) + ": " + message);
  std::cerr << "[ERROR] " << status.message() << std::endl;
  return status;
}

const Token *const SQLParser::current() { return &this->tokens[this->ptr]; }

bool SQLParser::checkCurrentType(Token::TokenType type) {
  return current()->type == type;
}

bool SQLParser::match(Token::TokenType type) {
  if (ptr >= tokens.size() || tokens[ptr].type != type) {
    return false;
  }
  ptr++;
  return true;
}

absl::StatusOr<Token::TokenType>
SQLParser::multiMatch(const std::vector<Token::TokenType> &types) {
  if (ptr >= tokens.size()) {
    return absl::InvalidArgumentError("Invalid syntax");
  }
  if (std::find(types.begin(), types.end(), current()->type) == types.end()) {
    return absl::InvalidArgumentError("Invalid syntax");
  }

  return eat()->type;
}

OptionalNode SQLParser::createStmt() {
  switch (current()->type) {
  case (Token::TOKEN_DATABASE): {
    this->eat(); // DATABASE
    if (!checkCurrentType(Token::TOKEN_IDENTIFIER)) {
      std::cerr << "[ERROR] Line " << current()->line << ": "
                << "Expect identifier after DATABASE" << std::endl;
    return absl::InvalidArgumentError("Invalid syntax");
    }

    return std::unique_ptr<ASTNode>(new CreateStmt(
        std::unique_ptr<VerdantObject>(new DatabaseNode(eat()->value))));
  }
  case (Token::TOKEN_TABLE): {
    this->eat(); // TABLE
    size_t numPrimary = 0;
    if (current()->type != Token::TOKEN_IDENTIFIER) {
      std::cerr << "[ERROR] Line " << current()->line << ": "
                << "Expect identifier after TABLE" << std::endl;
      return absl::InvalidArgumentError("Invalid syntax");
    }
    std::unique_ptr<TableNode> table(new TableNode(eat()->value));
    if (!this->consume(Token::TOKEN_LEFT_PAREN, "Expected '(' after table identifier").ok()) {
      return absl::InvalidArgumentError("Invalid syntax");
    }
    size_t currentFieldIdx = 0;
    while (this->peek().ok() &&
           this->peek().value()->type != Token::TOKEN_RIGHT_PAREN) {
      auto optionalName = consume(Token::TOKEN_IDENTIFIER,
                                  "Expect identifier after '(' or ','");
      if (!optionalName.ok()) {
        return absl::InvalidArgumentError("Invalid syntax");
      }
      std::string name = optionalName.value()->value;
      absl::StatusOr<const Token *> optionalTypeToken = multiConsume(
          {Token::TOKEN_VARCHAR, Token::TOKEN_INT, Token::TOKEN_FLOAT},
          "Expect type after column identifier");
      if (!optionalTypeToken.ok()) {
        return absl::InvalidArgumentError("Invalid syntax");
      }
      const Token* typeToken = optionalTypeToken.value();
      ColumnInfo::ColumnType type;
      switch (typeToken->type) {
      case Token::TOKEN_VARCHAR:
        type = ColumnInfo::VARCHAR;
        break;
      case Token::TOKEN_INT:
        type = ColumnInfo::INT;
        break;
      case Token::TOKEN_FLOAT:
        type = ColumnInfo::FLOAT;
        break;
      default:
        std::cerr << "[ERROR] Unreachable" << std::endl;
        Utility::handleFatalStatus(absl::InternalError("Unreachable"));
      }
      size_t length = 0;
      if (typeToken->type == Token::TOKEN_VARCHAR) {
        if (!consume(Token::TOKEN_LEFT_PAREN, "Expect '(' after VARCHAR").ok()) {
          return absl::InvalidArgumentError("Invalid syntax");
        }
        auto optionalLength =
            consume(Token::TOKEN_INT_VALUE, "Expect integer after '('");
        if (!optionalLength.ok()) {
          return absl::InvalidArgumentError("Invalid syntax");
        }
        sscanf(optionalLength.value()->value.c_str(), "%zu", &length);
        if (!consume(Token::TOKEN_RIGHT_PAREN, "Expect ')' after integer").ok()) {
          return absl::InvalidArgumentError("Invalid syntax");
        }
      }
      auto isPrimaryKey = false;
      if (match(Token::TOKEN_PRIMARY)) {
        if (!consume(Token::TOKEN_KEY, "Expect 'KEY' after 'PRIMARY'").ok()) {
          return absl::InvalidArgumentError("Invalid syntax");
        }
        isPrimaryKey = true;
        numPrimary++;
      }
      ColumnInfo info = {type, length, isPrimaryKey};
      bool addResult =
          table->addColumn(name, currentFieldIdx++, std::move(info));
      if (!addResult) {
          return absl::InvalidArgumentError("Invalid syntax");
      }
      if (!this->multiConsume({Token::TOKEN_RIGHT_PAREN, Token::TOKEN_COMMA},
                              "Expect ')' or ',' after column declaration").ok()) {
        return absl::InvalidArgumentError("Invalid syntax");
      }
    }
    this->match(Token::TOKEN_RIGHT_PAREN); // Optional ',' before ')'
    if (numPrimary > 1) {
      std::cerr << "[ERROR] Too many primary key columns declared" << std::endl;
      return absl::InvalidArgumentError("Invalid syntax");
    }
    return std::unique_ptr<ASTNode>(new CreateStmt(std::move(table)));
  }
  default:
    return this->error("Invalid token " + this->current()->value);
  }
}

OptionalNode SQLParser::stmt() {
  switch (current()->type) {
  case (Token::TOKEN_CREATE): {
    this->eat();
    return this->createStmt();
  }
  default:
    std::cerr << "[ERROR] Invalid token: '" << current()->value << "'"
              << std::endl;
    return absl::InvalidArgumentError("Invalid syntax");
  }
}

absl::StatusOr<AST> SQLParser::parse() {
  this->ptr = 0;
  AST ast;

  while (this->ptr < tokens.size()) {
    if (tokens[this->ptr].type == Token::TOKEN_SEMICOLON) {
      this->ptr++;
      continue;
    }
    OptionalNode nodeOrError = this->stmt();
    if (!nodeOrError.ok()) {
      return nodeOrError.status();
    }
    std::unique_ptr<ASTNode> node = std::move(nodeOrError.value());

    ast.addRoot(std::move(node));

    if (this->ptr < tokens.size() and
        tokens[this->ptr].type != Token::TOKEN_SEMICOLON) {
      return this->error("Invalid token at the end: " + current()->value);
    }
  }

  return ast;
}
