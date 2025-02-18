#include "create_stmt.h"

#include "visitor.h"
#include <memory>
#include <utility>

CreateStmt::CreateStmt(std::unique_ptr<VerdantObject> creation) : creation(std::move(creation)) {}

void CreateStmt::accept(Visitor* visitor) {
  visitor->visit(this);
}

