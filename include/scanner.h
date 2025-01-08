#pragma once

#include "token.h"
#include <vector>
#include <string>

#include "optional.h"

std::ostream& operator<<(std::ostream& os, const Token& obj);

class Scanner {
private:
  size_t ptr;
  size_t line;
  const std::string& text;

  void skipBlank();

public:
  Scanner(const std::string& text);
  Optional<std::vector<Token>> scan();
};
