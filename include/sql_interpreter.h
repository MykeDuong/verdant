#pragma once

#include "visitor.h"
#include "context.h"

#include <ast.h>
#include <status.h>

class SQLInterpreter final: public Visitor {
private:
  VerdantStatus::StatusEnum status;
  void visit(const CreateStmt* node);
  void visit(const DatabaseNode* node);
  void visit(const TableNode* node);
  const AST& ast;
  Context& context;

public:
  SQLInterpreter(const AST& ast, Context& context);
  VerdantStatus::StatusEnum interpret();
};
