#pragma once

#include <string>

#include "ast_node.h"

struct VerdantObject: public ASTNode {
  virtual const std::string& getName() const = 0;
};
