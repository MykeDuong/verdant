#pragma once

#include <string>
#include "optional.hpp"

struct Context {
  Optional<std::string> database;
  Optional<std::string*> statement;
};
