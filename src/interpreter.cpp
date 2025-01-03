#include "interpreter.h"
#include "create_stmt.h"
#include "status.h"
#include "parameters.h"
#include "util.h"
#include <iostream>

#include <fstream>

VerdantStatus::StatusEnum Interpreter::interpret(AST&& ast) {
  this->ast = std::move(ast);
  
  for (size_t i = 0; i < this->ast.roots.size(); i++) {
    auto root = this->ast.roots[i].get();
    root->accept(this);
    if (status != VerdantStatus::SUCCESS) {
      return status;
    }
  }

  return status;
}


void Interpreter::visit(const CreateStmt* node) {
  switch (node->type) {
    case CreateStmt::DATABASE :{
      std::string path = DATA_PATH + node->creationName + "/";
      bool result = Utility::createDirectory(path);
      if (!result) {
        std::cerr << "[ERROR] Verdant cannot create directory with path " + path << std::endl;
        this->status = VerdantStatus::INVALID_PERMISSION;
      }
      this->status = VerdantStatus::SUCCESS;
    }
  }
}
