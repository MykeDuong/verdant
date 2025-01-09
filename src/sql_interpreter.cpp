#include "sql_interpreter.h"
#include "create_stmt.h"
#include "database_node.h"
#include "status.h"
#include "parameters.h"
#include "util.h"

#include <iostream>

SQLInterpreter::SQLInterpreter(const AST& ast, Context& context) : ast(ast), context(context) {}

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


void SQLInterpreter::visit(const CreateStmt* node) {
  switch (node->type) {
    case CreateStmt::DATABASE :{
      std::string path = DATA_PATH + node->creation->getName() + "/";
      bool result = Utility::createDirectory(path);
      if (!result) {
        std::cerr << "[ERROR] Verdant cannot create directory with path " + path << std::endl;
        this->status = VerdantStatus::INVALID_PERMISSION;
        return;
      }
      
      this->status = VerdantStatus::SUCCESS;
      return;
    }
    case CreateStmt::TABLE :{
      if (!context.database.unwrappable()) {
        std::cerr << "[ERROR] No database currently connected" << std::endl;
        this->status = VerdantStatus::UNSPECIFIED_DATABASE;
        return;
      }
      this->status = VerdantStatus::UNIMPLEMENTED;
      return;
    }
  }
}

void SQLInterpreter::visit(const DatabaseNode* node) {}

void SQLInterpreter::visit(const TableNode* node) {
  status = VerdantStatus::UNIMPLEMENTED;
}
