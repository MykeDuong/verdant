#pragma once

#include "ast.h"
#include "ast_node.h"
#include "optional.h"
#include "token.h"
#include "object_database.h"
#include <memory>
#include <vector>

typedef Optional<std::unique_ptr<ASTNode>> OptionalNode;

class Parser {
private:
  size_t ptr;
  
  size_t eat();
  size_t consume(const std::vector<Token>& tokens, Token::TokenType type);

  OptionalNode error(const Token& token);
  OptionalNode stmt(const std::vector<Token>& tokens);
  OptionalNode createStmt(const std::vector<Token>& tokens);
  Optional<std::unique_ptr<ObjectDatabase>> objectDatabase(const std::vector<Token>& tokens);

public:
  Parser();
  Optional<AST> parse(const std::vector<Token>& tokens);
};
