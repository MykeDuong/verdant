#pragma once

#include <string>
#include <memory>

#include "stmt.h"
#include "verdant_object.h"

struct CreateStmt: public Stmt {
  typedef enum {
    DATABASE,
    TABLE,
//  INDEX,
  } CreationType;

  const CreationType type;
  std::unique_ptr<VerdantObject> creation;

  CreateStmt(CreationType type, std::unique_ptr<VerdantObject> creation);
  void accept(Visitor* visitor);
};

