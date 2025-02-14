#pragma once

#include "token.hpp"
#include "status.hpp"
#include "optional.hpp"
#include "context.hpp"
#include <vector>

class CommandInterpreter {
private:
  Context& context;
  const std::vector<Token>& tokens;
  size_t ptr;

  Optional<const Token*> consume(Token::TokenType type, const std::string& message);
  const Token* const eat();
  const Token* const current() const;
  Optional<const Token*> peek() const;
  void error(const std::string& message) const;
  bool isAtEnd() const;

public:
  CommandInterpreter(Context& context, const std::vector<Token>& tokens);
  VerdantStatus::StatusEnum interpret();
};
