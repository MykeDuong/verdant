#pragma once

#include "absl/status/statusor.h"
#include "ast.hpp"
#include "ast_node.hpp"
#include "token.hpp"

#include <memory>
#include <vector>

typedef absl::StatusOr<std::unique_ptr<ASTNode>> OptionalNode;

class SQLParser {
private:
  size_t ptr;
  const std::vector<Token>& tokens;
  
  absl::StatusOr<const Token*> consume(Token::TokenType type, const std::string& message);
  absl::StatusOr<const Token*> multiConsume(const std::vector<Token::TokenType>& types, const std::string& message);
  const Token* const eat();
  const Token* const current();
  absl::StatusOr<const Token*> peek() const;
  absl::Status error(const std::string& message);
  OptionalNode stmt();
  OptionalNode createStmt();
  bool match(Token::TokenType type);
  absl::StatusOr<Token::TokenType> multiMatch(const std::vector<Token::TokenType>& types);
  bool checkCurrentType(Token::TokenType);

public:
  SQLParser(const std::vector<Token>& tokens);
  absl::StatusOr<AST> parse();
};
