#include "ast.hpp"

void AST::addRoot(std::unique_ptr<ASTNode> node) {
  roots.push_back(std::move(node));
}
