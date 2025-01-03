#pragma once

#include "ast.h"
#include "visitor.h"

class ASTPrinter: public Visitor {
private:
  size_t skipSpace;
  std::string lineStart;
  bool isRoot;
  void printLineStart();
  void visit(const CreateStmt* node);

public:
  ASTPrinter();
  ASTPrinter(std::string lineStart);
  void print(AST& ast);
};
