#pragma once

#include <string>

struct Token {
  typedef enum {
    TOKEN_CREATE,
    TOKEN_DATABASE,
    TOKEN_IDENTIFIER,
    TOKEN_SEMICOLON,
  } TokenType;

  const TokenType type;
  const std::string value;
  const int line;

  Token(TokenType type, std::string value, int line);
};
