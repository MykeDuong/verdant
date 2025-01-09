#include "ast_printer.h"
#include "create_stmt.h"
#include "table_node.h"
#include <iostream>
#include <string>

ASTPrinter::ASTPrinter(): skipSpace(0), lineStart("") {
}

ASTPrinter::ASTPrinter(std::string lineStart): skipSpace(0), lineStart(lineStart) {
}

void ASTPrinter::printLineStart(bool isMiddle) {
  std::cout << this->lineStart;
  for (size_t i = 0; i < this->skipSpace; i++) {
    std::cout << " ";
  }
  if (!this->isRoot) {
    std::cout << (isMiddle ? "|--- " : "'--- ");
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
  std::cout << "CREATE\n";
  bool prevIsRoot = isRoot;
  isRoot = false;
  switch (node->type) {
    case CreateStmt::DATABASE:
      node->creation->accept(this);
      break;
    case CreateStmt::TABLE:
      node->creation->accept(this);
      break;
  }
  isRoot = prevIsRoot;
}

void ASTPrinter::visit(const DatabaseNode* node) {
  this->printLineStart(false);
  std::cout << "DATABASE { name: " << node->getName() << " }" << std::endl;
}

static std::string printColumnType(const ColumnInfo& column) {
  switch (column.type) {
    case ColumnInfo::INT:
      return "INT";
    case ColumnInfo::FLOAT:
      return "FLOAT";
    case ColumnInfo::VARCHAR:
      return "VARCHAR(" + std::to_string(column.varcharSize) + ")";
  }
  return "";
}

void ASTPrinter::visit(const TableNode* node) {
  this->printLineStart(false);
  std::cout << "TABLE { name: " << node->getName() << " }" << std::endl;
  this->skipSpace += 5;
  for (size_t i = 0; i < node->columns.size(); i++) {
    auto &column = node->columns[i];
    this->printLineStart(i != node->columns.size() - 1);
    std::cout << "{ name: " << column.name << ", type: " << printColumnType(column)
              << ", primary: " << (column.isPrimary ? "true" : "false") 
              << " }" << std::endl;

  }
  this->skipSpace -= 5;
}

