#include "scanner.hpp"
#include "status.hpp"
#include "util.hpp"
#include <algorithm>
#include <cctype>
#include <array>

std::ostream& operator<<(std::ostream& os, const Token& obj) {
  os << "{" << obj.type << ", " << obj.value << "}";
  return os;
}

Scanner::Scanner(const std::string& text) : ptr(0), line(1), text(text) {}

Optional<std::vector<Token>> Scanner::scan() {
  std::vector<Token> tokens;

  while (ptr < text.size()) {
    skipBlank();
    if (text[ptr] == ';') {
      tokens.push_back({ Token::TOKEN_SEMICOLON, ";", line });
      ptr++;
      continue;
    }
    if (text[ptr] == '(') {
      Token token = { Token::TOKEN_LEFT_PAREN, "(", line };
      tokens.push_back(token);
      ptr++;
      continue;
    }
    if (text[ptr] == ')') {
      Token token = { Token::TOKEN_RIGHT_PAREN, ")", line };
      tokens.push_back(token);
      ptr++;
      continue;
    }
    if (text[ptr] == ',') {
      Token token = { Token::TOKEN_COMMA, ",", line };
      tokens.push_back(token);
      ptr++;
      continue;
    }
    if (text[ptr] == '\\') {
      Token token = { Token::TOKEN_BACK_SLASH, ",", line };
      tokens.push_back(token);
      ptr++;
      continue;
    }

    if (std::isdigit(text[ptr])) {
      size_t endPtr = ptr;
      bool isFloat = false;
      constexpr std::array<char, 5> allowedCharacters = {' ', '\n', '\t', '(', ')'};
      while (endPtr < text.size() && std::find(allowedCharacters.begin(), allowedCharacters.end(), text[endPtr]) == allowedCharacters.end()) {
        char cur = text[endPtr];
        if (cur != '.' && !std::isdigit(cur)) {
          std::cerr << "[ERROR] Line " << line << ": unexpected character " << cur << std::endl;
          return Optional<typeof tokens>(VerdantStatus::INVALID_SYNTAX);
        }
        if (cur == '.') {
          if (isFloat) {
            std::cerr << "[ERROR] Line " << line << ": unexpected character " << cur << std::endl;
            return Optional<typeof tokens>(VerdantStatus::INVALID_SYNTAX);
          }
          isFloat = true;
        }
        endPtr++;
      }
      tokens.push_back({ isFloat ? Token::TOKEN_FLOAT_VALUE : Token::TOKEN_INT_VALUE, text.substr(ptr, endPtr - ptr), line});
      ptr = endPtr;
    } else if (Utility::isAlpha(text[ptr])) {
      size_t endPtr = ptr;
      while (endPtr < text.size() and std::isalnum(text[endPtr])) {
        endPtr++;
      }
  
      std::string tokenString = text.substr(ptr, endPtr - ptr);
      std::string lowerTokenString = Utility::toLower(tokenString);
  
      if (lowerTokenString == "create") {
        tokens.push_back({ Token::TOKEN_CREATE, tokenString, line });
      } else if (lowerTokenString == "database") {
        tokens.push_back({ Token::TOKEN_DATABASE, tokenString, line });
      } else if (lowerTokenString == "table") {
        tokens.push_back({ Token::TOKEN_TABLE, tokenString, line });
      } else if (lowerTokenString == "int") {
        tokens.push_back({ Token::TOKEN_INT, tokenString, line });
      } else if (lowerTokenString == "float") {
        tokens.push_back({ Token::TOKEN_FLOAT, tokenString, line });
      } else if (lowerTokenString == "varchar") {
        tokens.push_back({ Token::TOKEN_VARCHAR, tokenString, line });
      } else if (lowerTokenString == "primary") {
        tokens.push_back({ Token::TOKEN_PRIMARY, tokenString, line });
      } else if (lowerTokenString == "key") {
        tokens.push_back({ Token::TOKEN_KEY, tokenString, line });
      } else {
        tokens.push_back({ Token::TOKEN_IDENTIFIER, tokenString, line });
      }

      ptr = endPtr;
    } else {
      VerdantStatus::handleError(VerdantStatus::UNIMPLEMENTED);
    }
  }

  Optional<typeof tokens> returnValue = tokens;
  return returnValue;
}

void Scanner::skipBlank() {
  constexpr std::array<char, 3> blanks = {' ', '\t', '\n'};
  while (std::find(blanks.begin(), blanks.end(), text[ptr]) != blanks.end()) {
    if (text[ptr] == ' ' or text[ptr] == '\t') {
      ptr++;
      continue;
    }
    if (text[ptr] == '\n') {
      ptr++;
      line++;
      continue;
    }
  }
}
