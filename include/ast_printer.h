#pragma once

#include "ast.h"
#include "database_node.h"
#include "visitor.h"

class ASTPrinter: public Visitor {
private:
  size_t skipSpace;
  std::string lineStart;
  bool isRoot;
  void printLineStart(bool isMiddle = true);
  void visit(const CreateStmt* node);
  void visit(const DatabaseNode* node);
  void visit(const TableNode* node);

public:
  ASTPrinter();
  ASTPrinter(std::string lineStart);
  void print(AST& ast);
};
