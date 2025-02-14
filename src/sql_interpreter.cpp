#include "sql_interpreter.hpp"
#include "create_stmt.hpp"
#include "database_node.hpp"
#include "parameters.hpp"
#include "status.hpp"
#include "util.hpp"

#include <iostream>

SQLInterpreter::SQLInterpreter(const AST &ast, Context &context)
    : ast(ast), context(context) {}

VerdantStatus::StatusEnum SQLInterpreter::interpret() {
  for (size_t i = 0; i < this->ast.roots.size(); i++) {
    auto root = ast.roots[i].get();
    root->accept(this);
    if (status != VerdantStatus::SUCCESS) {
      return status;
    }
  }
  return status;
}

void SQLInterpreter::visit(const CreateStmt *node) {
  node->creation->accept(this);
}

void SQLInterpreter::visit(const TableNode *node) {
  if (!context.database.unwrappable()) {
    std::cerr << "[ERROR] No database currently connected" << std::endl;
    this->status = VerdantStatus::UNSPECIFIED_DATABASE;
    return;
  }

  this->status = VerdantStatus::UNIMPLEMENTED;
}

void SQLInterpreter::visit(const DatabaseNode *node) {
  std::string path = DATA_PATH + node->getName() + "/";
  if (Utility::isDirectoryExist(path)) {
    std::cerr << "[ERROR] Database already exist" << std::endl;
    this->status = VerdantStatus::INVALID_PERMISSION;
    return;
  }
  bool result = Utility::createDirectory(path);
  if (!result) {
    std::cerr << "[ERROR] Verdant cannot create directory with path " + path
              << std::endl;
    this->status = VerdantStatus::INVALID_PERMISSION;
    return;
  }

  this->status = VerdantStatus::UNIMPLEMENTED;
}
