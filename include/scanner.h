#pragma once

#include "token.h"
#include <vector>
#include <string>

#include "optional.h"

std::ostream& operator<<(std::ostream& os, const Token& obj);

struct Scanner {
  Scanner();
  Optional<std::vector<Token>> scan(std::string& statement);
};
