#pragma once

#include "ast_node.hpp"
#include <memory>
#include <vector>

class AST {
public:
  std::vector<std::unique_ptr<ASTNode>> roots;

  void addRoot(std::unique_ptr<ASTNode> node);
};
