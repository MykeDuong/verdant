#pragma once

#include "visitor.h"
#include <ast.h>
#include <status.h>

class Interpreter final: public Visitor {
private:
  VerdantStatus::StatusEnum status;
  void visit(const CreateStmt* node);
  AST ast;

public:
  VerdantStatus::StatusEnum interpret(AST&& ast);
};
