#include "create_stmt.h"

#include "visitor.h"
#include <memory>
#include <utility>

CreateStmt::CreateStmt(std::unique_ptr<Object> obj) : obj(std::move(obj)) {}

CreateStmt::~CreateStmt() {
  
}

void CreateStmt::accept(Visitor* visitor) {
  visitor->visit(this);
}

