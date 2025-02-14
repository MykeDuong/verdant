#pragma once

#include "ast.hpp"
#include "ast_node.hpp"
#include "optional.hpp"
#include "token.hpp"
#include <memory>
#include <vector>

typedef Optional<std::unique_ptr<ASTNode>> OptionalNode;

class SQLParser {
private:
  size_t ptr;
  const std::vector<Token>& tokens;
  
  Optional<const Token*> consume(Token::TokenType type, const std::string& message);
  Optional<const Token*> multiConsume(const std::vector<Token::TokenType>& types, const std::string& message);
  const Token* const eat();
  const Token* const current();
  Optional<const Token*> peek() const;
  OptionalNode error(const std::string& message);
  OptionalNode stmt();
  OptionalNode createStmt();
  bool match(Token::TokenType type);
  Optional<Token::TokenType> multiMatch(const std::vector<Token::TokenType>& types);
  bool checkCurrentType(Token::TokenType);

public:
  SQLParser(const std::vector<Token>& tokens);
  Optional<AST> parse();
};
