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
  std::string path =
      DATA_PATH + context.database.unwrap() + "/" + node->getName();
  bool result = Utility::createDirectory(path);
  if (!result) {
    std::cerr << "[ERROR] Verdant cannot create directory with path " + path
              << std::endl;
    this->status = VerdantStatus::INVALID_PERMISSION;
    return;
  }
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
  Columns columns;
  columns["name"] = {0, {ColumnInfo::VARCHAR, MAX_OBJECT_NAME, true}};
  columns["type"] = {1, {ColumnInfo::INT, 0, false}};

  std::string masterTableName = node->getName() + "_verdant_master.vtbl";
  Table verdantMaster(node->getName(), masterTableName, std::move(columns));
  std::vector<Field> record;
  record.push_back({"name", masterTableName});
  record.push_back({"type", std::to_string(VerdantObjectType::TABLE)});
  verdantMaster.addRecord(record);

  verdantMaster.save();

  this->status = VerdantStatus::SUCCESS;
}
