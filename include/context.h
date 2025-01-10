#pragma once

#include <string>
#include "optional.h"

struct Context {
  Optional<std::string> database;
  Optional<std::string*> statement;
};
