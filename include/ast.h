#pragma once

#include "ast_node.h"
#include <memory>
#include <vector>

class AST {
public:
  std::vector<std::unique_ptr<ASTNode>> roots;

  void addRoot(std::unique_ptr<ASTNode> node);
};
