#pragma once

#include "absl/status/status.h"
#include "context.hpp"
#include "visitor.hpp"

#include "ast.hpp"

class SQLInterpreter final : public Visitor {
private:
  absl::Status status;
  void visit(const CreateStmt *node);
  void visit(const DatabaseNode *node);
  void visit(const TableNode *node);
  const AST &ast;
  Context &context;

public:
  SQLInterpreter(const AST &ast, Context &context);
  absl::Status interpret();
};
