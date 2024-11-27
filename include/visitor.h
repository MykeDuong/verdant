#ifndef VISITOR_H
#define VISITOR_H

#include "create_stmt.h"
#include "object_database.h"

struct Visitor {
  virtual void visit(const CreateStmt* node) = 0;
  virtual void visit(const ObjectDatabase* node) = 0;
};

#endif
