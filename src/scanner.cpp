#include "scanner.h"
#include "util.h"
#include <vector>

std::ostream& operator<<(std::ostream& os, const Token& obj) {
      os << "{" << obj.type << ", " << obj.value << "}";
      return os;
}

Scanner::Scanner() {}

Optional<std::vector<Token>> Scanner::scan(std::string& text) {
  int line = 1;
  std::vector<Token> tokens;

  size_t ptr = 0;

  while (ptr < text.size()) {
    if (text[ptr] == ';') {
      Token token = { Token::TOKEN_SEMICOLON, ";", line };
      tokens.push_back(token);
      ptr++;
      continue;
    }
    if (text[ptr] == ' ' or text[ptr] == '\t') {
      ptr++;
      continue;
    }
    if (text[ptr] == '\n') {
      ptr++;
      line++;
      continue;
    }
    size_t endPtr = ptr;
    while (endPtr < text.size() and text[endPtr] != ' ' and text[endPtr] != '\t' and text[endPtr] != ';') {
      endPtr++;
    }

    std::string tokenString = text.substr(ptr, endPtr - ptr);
    std::string lowerTokenString = Utility::toLower(tokenString);

    if (lowerTokenString == "create") {
      tokens.push_back({ Token::TOKEN_CREATE, tokenString, line });
    } else if (lowerTokenString == "database") {
      tokens.push_back({ Token::TOKEN_DATABASE, tokenString, line });
    } else {
      tokens.push_back({ Token::TOKEN_IDENTIFIER, tokenString, line });
    }
    
    ptr = endPtr;
  }

  Optional<typeof tokens> returnValue = tokens;
  return returnValue;
}
