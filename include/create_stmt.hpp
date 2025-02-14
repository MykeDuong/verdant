#pragma once

#include <string>
#include <memory>

#include "stmt.hpp"
#include "verdant_object.hpp"

struct CreateStmt: public Stmt {
  std::unique_ptr<VerdantObject> creation;

  CreateStmt(std::unique_ptr<VerdantObject> creation);
  void accept(Visitor* visitor);
};

