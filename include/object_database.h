#pragma once

#include "object.h"
#include <string>

struct ObjectDatabase: public Object {
  std::string identifier;
  void accept(Visitor* visitor);
  ObjectDatabase(std::string identifier);
  ~ObjectDatabase();
};
