#pragma once

#include <vector>
#include <string>

#include "optional.h"

class Scanner {
public:
  Scanner();
  Optional<std::vector<std::string>> parse(std::string& statement);
};
