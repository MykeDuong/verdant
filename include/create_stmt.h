#pragma once

#include "stmt.h"
#include <string>


struct CreateStmt: public Stmt {
  typedef enum {
    DATABASE,
//  TABLE,
//  INDEX,
  } CreationType;
  
  const CreationType type;
  const std::string creationName;

  CreateStmt(CreationType type, std::string creationName);
  void accept(Visitor* visitor);
};

