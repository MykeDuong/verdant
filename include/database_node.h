#pragma once

#include "verdant_object.h"

class DatabaseNode final: public VerdantObject {
private:
  const std::string name;

public:
  DatabaseNode(const std::string& name);
  const std::string& getName() const;
  void accept(Visitor* visitor);
};
