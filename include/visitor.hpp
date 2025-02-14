#ifndef VISITOR_H
#define VISITOR_H

#include "create_stmt.hpp"
#include "database_node.hpp"
#include "table_node.hpp"

struct Visitor {
  virtual void visit(const CreateStmt* node) = 0;
  virtual void visit(const DatabaseNode* node) = 0;
  virtual void visit(const TableNode* node) = 0;
};

#endif
