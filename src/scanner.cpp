#include "scanner.h"
#include <vector>

Scanner::Scanner() {}

Optional<std::vector<std::string>> Scanner::parse(std::string& statement) {
  std::vector<std::string> tokens;

  size_t ptr = 0;

  while (ptr < statement.size()) {
    if (statement[ptr] == ' ' or statement[ptr] == '\t') {
      ptr++;
      continue;
    }
    size_t endPtr = ptr;
    while (endPtr < statement.size() and statement[endPtr] != ' ' and statement[endPtr] != '\t') {
      endPtr++;
    }

    tokens.push_back(statement.substr(ptr, endPtr - ptr));
    ptr = endPtr;
  }

  Optional<typeof tokens> returnValue = tokens;
  return returnValue;
}
