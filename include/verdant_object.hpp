#pragma once

#include <string>

#include "ast_node.hpp"

typedef enum {
  DATABASE,
  TABLE,
//INDEX,
} VerdantObjectType;

struct VerdantObject: public ASTNode {
  virtual const std::string& getName() const = 0;
  virtual const VerdantObjectType getType() const = 0;
};
