#pragma once

#include <string>
#include <memory>

#include "stmt.h"
#include "verdant_object.h"

struct CreateStmt: public Stmt {
  std::unique_ptr<VerdantObject> creation;

  CreateStmt(std::unique_ptr<VerdantObject> creation);
  void accept(Visitor* visitor);
};

