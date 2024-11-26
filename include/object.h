#pragma once

#include "ast_node.h"

struct Object: public ASTNode {
  typedef enum {
    OBJECT_DATABASE,
    OBJECT_TABLE,
  } ObjectType;

  ObjectType type;

  virtual ~Object() = 0;
};
