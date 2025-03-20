#pragma once

#include "absl/status/statusor.h"
#include "token.hpp"
#include <vector>
#include <string>

std::ostream& operator<<(std::ostream& os, const Token& obj);

class Scanner {
private:
  size_t ptr;
  size_t line;
  const std::string& text;

  void skipBlank();

public:
  Scanner(const std::string& text);
  absl::StatusOr<std::vector<Token>> scan();
};
