#include "sql_interpreter.hpp"
#include "absl/status/status.h"
#include "create_stmt.hpp"
#include "database_node.hpp"
#include "file_operator.hpp"
#include "parameters.hpp"
#include "util.hpp"
#include "absl/strings/string_view.h"

#include <cstdlib>
#include <iostream>

SQLInterpreter::SQLInterpreter(const AST &ast, Context &context)
    : ast(ast), context(context) {}

absl::Status SQLInterpreter::interpret() {
  for (size_t i = 0; i < this->ast.roots.size(); i++) {
    auto root = ast.roots[i].get();
    root->accept(*this);
    if (status.ok()) {
      return status;
    }
  }
  return status;
}

void SQLInterpreter::visit(const CreateStmt *node) {
  node->creation->accept(*this);
}

void SQLInterpreter::visit(const TableNode *node) {
  if (!context.database.ok()) {
    std::cerr << "[ERROR] No database currently connected" << std::endl;
    this->status = absl::FailedPreconditionError("No database currently connected");
    return;
  }

  FileOperator& op = FileOperator::getFileOperator(Parameter::DATA_PATH + context.database.value());

  
  absl::StatusOr<std::unique_ptr<char[]>> optionalFirstPage = op.readPage(0);
  std::unique_ptr<char[]> firstPage = std::move(optionalFirstPage.value());

  this->status = absl::UnimplementedError("Table creation unimplemented");
}

void SQLInterpreter::visit(const DatabaseNode *node) {
  std::string path = Parameter::DATA_PATH + node->getName();
  std::cout << "Creating database at location " << path << std::endl;
  if (Utility::isFileExist(path)) {
    std::cerr << "[ERROR] Database already exist" << std::endl;
    this->status = absl::PermissionDeniedError("Database already exist");
    return;
  }
  bool result = Utility::createDirectory(Parameter::DATA_PATH);
  if (!result) {
    std::cerr << "[ERROR] Verdant cannot create directory with path " + path
              << std::endl;
    this->status = absl::PermissionDeniedError("Cannot create directory with path " + path);
    return;
  }
  FileOperator &fileOp = FileOperator::getFileOperator(path);
  char *data = (char *)(std::calloc(sizeof(char), Parameter::BLOCK_SIZE));
  data[0] = 1;
  absl::StatusOr<std::size_t> firstPage = fileOp.writeNewPage(data);
  absl::StatusOr<std::size_t> secondPage = fileOp.writeNewPage(data);
  assert(firstPage.value() == 0);
  assert(secondPage.value() == 1);

  this->status = absl::OkStatus();
}
