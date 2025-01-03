#include "create_stmt.h"

#include "visitor.h"
#include <memory>
#include <utility>

CreateStmt::CreateStmt(const CreationType type, const std::string creationName) : type(type), creationName(creationName) {
}

void CreateStmt::accept(Visitor* visitor) {
  visitor->visit(this);
}

