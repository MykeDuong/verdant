#include "ast_printer.h"
#include "create_stmt.h"
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
  std::cout << "CREATE ";
  switch (node->type) {
    case CreateStmt::DATABASE:
      std::cout << "{ type = DATABASE }, " << "{ name = " << node->creationName << " }" << std::endl;

      break;
  }

}
