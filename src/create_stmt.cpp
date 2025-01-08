#include "create_stmt.h"

#include "visitor.h"
#include <memory>
#include <utility>

CreateStmt::CreateStmt(CreationType type, std::unique_ptr<VerdantObject> creation) : type(type), creation(std::move(creation)) {}

void CreateStmt::accept(Visitor* visitor) {
  visitor->visit(this);
}

