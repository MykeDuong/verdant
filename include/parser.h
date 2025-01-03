#pragma once

#include "ast.h"
#include "ast_node.h"
#include "optional.h"
#include "token.h"
#include <memory>
#include <vector>

typedef Optional<std::unique_ptr<ASTNode>> OptionalNode;

class Parser {
private:
  size_t ptr;
  const std::vector<Token>& tokens;
  
  size_t eat();

  OptionalNode error();
  OptionalNode stmt();
  OptionalNode createStmt();

public:
  Parser(const std::vector<Token>& tokens);
  Optional<AST> parse();
};
