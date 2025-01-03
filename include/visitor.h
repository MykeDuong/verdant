#ifndef VISITOR_H
#define VISITOR_H

#include "create_stmt.h"

struct Visitor {
  virtual void visit(const CreateStmt* node) = 0;
};

#endif
