#include "sql_interpreter.h"
#include "create_stmt.h"
#include "database_node.h"
#include "parameters.h"
#include "status.h"
#include "table.h"
#include "util.h"

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
  
  std::unique_ptr<Table> masterTable = Table::getMasterTable(context.database.peek());
  std::vector<Field> record;
  record.push_back({"name", node->getName()});
  record.push_back({"type", std::to_string(VerdantObjectType::TABLE)});
  record.push_back({"create_statement", *context.statement.peek()});
  masterTable->addRecord(record);
  Columns columns = std::move(node->columns);

  Table newTable(&context, node->getName(), std::move(columns));

  masterTable->save();
  newTable.save();

  this->status = VerdantStatus::SUCCESS;
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
  // Create verdant_master table
  Table::createMasterTable(node->getName());

  this->status = VerdantStatus::SUCCESS;
}
