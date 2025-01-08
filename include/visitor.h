#ifndef VISITOR_H
#define VISITOR_H

#include "create_stmt.h"
#include "database_node.h"
#include "table_node.h"

struct Visitor {
  virtual void visit(const CreateStmt* node) = 0;
  virtual void visit(const DatabaseNode* node) = 0;
  virtual void visit(const TableNode* node) = 0;
};

#endif
