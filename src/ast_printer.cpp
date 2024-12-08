#include "ast_printer.h"
#include <iostream>

ASTPrinter::ASTPrinter(): skipSpace(0), lineStart("") {
}

ASTPrinter::ASTPrinter(std::string lineStart): skipSpace(0), lineStart(lineStart) {
}

void ASTPrinter::printLineStart() {
  std::cout << this->lineStart;
  for (size_t i = 0; i < this->skipSpace; i++) {
    std::cout << " ";
  }
  if (!this->isRoot) {
    std::cout << "'--- ";
  }
}

void ASTPrinter::print(AST& ast) {
  for (auto&& node: ast.roots) {
    this->isRoot = true;
    node->accept(this);
  }
}

void ASTPrinter::visit(const CreateStmt* node) {
  this->printLineStart();
  std::cout << "Create" << std::endl;
  this->isRoot = false;
  node->obj->accept(this);
}

void ASTPrinter::visit(const ObjectDatabase* node) {
  this->printLineStart();
  std::cout << "Database " << node->identifier << std::endl;
  this->isRoot = false;
}
