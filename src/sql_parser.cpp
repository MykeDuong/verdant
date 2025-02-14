#include "sql_parser.hpp"
#include "ast_node.hpp"
#include "create_stmt.hpp"
#include "status.hpp"
#include "database_node.hpp"
#include "table_node.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>

SQLParser::SQLParser(const std::vector<Token>& tokens) : tokens(tokens) {}

const Token* const SQLParser::eat() {
  return &tokens[this->ptr++];
}

Optional<const Token*> SQLParser::consume(Token::TokenType type, const std::string& message) {
  if (ptr >= tokens.size() || tokens[ptr].type != type) {
    error(message);
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }
  return eat();
}

Optional<const Token*> SQLParser::multiConsume(const std::vector<Token::TokenType>& types, const std::string& message) {
  if (ptr >= tokens.size()) {
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }
  if (std::find(types.begin(), types.end(), current()->type) == types.end()) {
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }

  return eat();
}

Optional<const Token*> SQLParser::peek() const {
  if (this->ptr + 1 >= tokens.size()) {
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }
  return &tokens[this->ptr];
}

OptionalNode SQLParser::error(const std::string& message) {
  std::cerr << "[ERROR] Line " << tokens[this->ptr].line << ": " << message << std::endl;
  return OptionalNode(nullptr);
}

const Token* const SQLParser::current() {
  return &this->tokens[this->ptr];
}

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

Optional<Token::TokenType> SQLParser::multiMatch(const std::vector<Token::TokenType>& types) {
  if (ptr >= tokens.size()) {
    return Optional<Token::TokenType>(VerdantStatus::INVALID_SYNTAX);
  }
  if (std::find(types.begin(), types.end(), current()->type) == types.end()) {
    return Optional<Token::TokenType>(VerdantStatus::INVALID_SYNTAX);
  }

  return eat()->type;
}

OptionalNode SQLParser::createStmt() {
  switch (current()->type) {
    case (Token::TOKEN_DATABASE): {
      this->eat(); // DATABASE
      if (!checkCurrentType(Token::TOKEN_IDENTIFIER)) {
        std::cerr << "[ERROR] Line " << current()->line << ": " << "Expect identifier after DATABASE" << std::endl;
        return OptionalNode(VerdantStatus::INVALID_SYNTAX);
      }

      return std::unique_ptr<ASTNode>(new CreateStmt(std::unique_ptr<VerdantObject>(new DatabaseNode(eat()->value))));
    }
    case (Token::TOKEN_TABLE): {
      this->eat(); // TABLE
      size_t numPrimary = 0;
      if (current()->type != Token::TOKEN_IDENTIFIER) {
        std::cerr << "[ERROR] Line " << current()->line << ": " << "Expect identifier after TABLE" << std::endl;
        return OptionalNode(VerdantStatus::INVALID_SYNTAX);
      }
      std::unique_ptr<TableNode> table(new TableNode(eat()->value));
      if (!this->consume(Token::TOKEN_LEFT_PAREN, "Expected '(' after table identifier").unwrappable()) {
        return OptionalNode(VerdantStatus::INVALID_SYNTAX);
      }
      size_t currentFieldIdx = 0;
      while (this->peek().unwrappable() && this->peek().unwrap()->type != Token::TOKEN_RIGHT_PAREN) {
        auto optionalName = consume(Token::TOKEN_IDENTIFIER, "Expect identifier after '(' or ','");
        if (!optionalName.unwrappable()) {
          return VerdantStatus::INVALID_SYNTAX;
        }
        std::string name  = optionalName.unwrap()->value;
        auto optionalTypeToken = multiConsume({ Token::TOKEN_VARCHAR, Token::TOKEN_INT, Token::TOKEN_FLOAT }, "Expect type after column identifier");
        if (!optionalTypeToken.unwrappable()) {
          return VerdantStatus::INVALID_SYNTAX;
        }
        auto typeToken = optionalTypeToken.unwrap();
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
            VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
        }
        size_t length = 0;
        if (typeToken->type == Token::TOKEN_VARCHAR) {
          if (!consume(Token::TOKEN_LEFT_PAREN, "Expect '(' after VARCHAR").unwrappable()) {
            return VerdantStatus::INVALID_SYNTAX;
          }
          auto optionalLength = consume(Token::TOKEN_INT_VALUE, "Expect integer after '('");
          if (!optionalLength.unwrappable()) {
            return VerdantStatus::INVALID_SYNTAX;
          }
          sscanf(optionalLength.unwrap()->value.c_str(), "%zu", &length);
          if (!consume(Token::TOKEN_RIGHT_PAREN, "Expect ')' after integer").unwrappable()) {
            return VerdantStatus::INVALID_SYNTAX;
          }
        }
        auto isPrimaryKey = false;
        if (match(Token::TOKEN_PRIMARY)) {
          if (!consume(Token::TOKEN_KEY, "Expect 'KEY' after 'PRIMARY'").unwrappable()) {
            return VerdantStatus::INVALID_SYNTAX;
          }
          isPrimaryKey = true;
          numPrimary++;
        } 
        ColumnInfo info = { type, length, isPrimaryKey };
        bool addResult = table->addColumn(name, currentFieldIdx++, std::move(info));
        if (!addResult) {
          return OptionalNode(VerdantStatus::INVALID_SYNTAX);
        }
        if (!this->multiConsume({ Token::TOKEN_RIGHT_PAREN, Token::TOKEN_COMMA}, "Expect ')' or ',' after column declaration").unwrappable()) {
          return OptionalNode(VerdantStatus::INVALID_SYNTAX);
        }
      }
      this->match(Token::TOKEN_RIGHT_PAREN); // Optional ',' before ')'
      if (numPrimary > 1) {
        std::cerr << "[ERROR] Too many primary key columns declared" << std::endl;
        return OptionalNode(VerdantStatus::INVALID_SYNTAX);
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
      std::cerr << "[ERROR] Invalid token: '" << current()->value << "'" << std::endl;
      return OptionalNode(VerdantStatus::INVALID_SYNTAX);
  }
}


Optional<AST> SQLParser::parse() {
  this->ptr = 0;
  AST ast;

  while (this->ptr < tokens.size()) {
    if (tokens[this->ptr].type == Token::TOKEN_SEMICOLON) {
      this->ptr++;
      continue;
    }
    OptionalNode nodeOrError = this->stmt();
    if (!nodeOrError.unwrappable()) {
      return Optional<AST>(nodeOrError.status);
    }
    std::unique_ptr<ASTNode> node = nodeOrError.unwrap();

    ast.addRoot(std::move(node));

    if (this->ptr < tokens.size() and tokens[this->ptr].type != Token::TOKEN_SEMICOLON) {
      this->error("Invalid token at the end: " + current()->value);
      return Optional<AST>();
    }
  }

  return Optional<AST>(std::move(ast));
}
