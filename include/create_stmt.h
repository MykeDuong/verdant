#pragma once

#include "stmt.h"
#include "object.h"
#include <memory>

struct CreateStmt: public Stmt {
  std::unique_ptr<Object> obj;
  void accept(Visitor* visitor);
  CreateStmt(std::unique_ptr<Object> obj);
  ~CreateStmt();
};

