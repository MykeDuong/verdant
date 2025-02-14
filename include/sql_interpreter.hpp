#pragma once

#include "context.hpp"
#include "visitor.hpp"

#include "ast.hpp"
#include "status.hpp"

class SQLInterpreter final : public Visitor {
private:
  VerdantStatus::StatusEnum status;
  void visit(const CreateStmt *node);
  void visit(const DatabaseNode *node);
  void visit(const TableNode *node);
  const AST &ast;
  Context &context;

public:
  SQLInterpreter(const AST &ast, Context &context);
  VerdantStatus::StatusEnum interpret();
};
