#include "token.hpp"
#include <string>

Token::Token(TokenType type, std::string value, size_t line) : type(type), value(value), line(line) {}

std::string Token::toString() {
  std::string result = "{ type: " + std::to_string(type) + ", value: " + value + ", line: " + std::to_string(line) + " }";

  return result;
}

