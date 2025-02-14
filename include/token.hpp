#pragma once

#include <string>

struct Token {
  typedef enum {
    TOKEN_CREATE,
    TOKEN_DATABASE,
    TOKEN_TABLE,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_VARCHAR,
    TOKEN_PRIMARY,
    TOKEN_KEY,
    TOKEN_INT_VALUE,
    TOKEN_FLOAT_VALUE,
    TOKEN_IDENTIFIER,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_BACK_SLASH,
  } TokenType;

  const TokenType type;
  const std::string value;
  const size_t line;

  Token(TokenType type, std::string value, size_t line);
  std::string toString();
};
