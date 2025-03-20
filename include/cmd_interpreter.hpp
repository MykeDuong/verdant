#pragma once

#include "absl/status/statusor.h"
#include "token.hpp"
#include "context.hpp"
#include <vector>

class CommandInterpreter {
private:
  Context& context;
  const std::vector<Token>& tokens;
  size_t ptr;

  absl::StatusOr<const Token*> consume(Token::TokenType type, const std::string& message);
  const Token* const eat();
  const Token* const current() const;
  absl::StatusOr<const Token*> peek() const;
  void error(const std::string& message) const;
  bool isAtEnd() const;

public:
  CommandInterpreter(Context& context, const std::vector<Token>& tokens);
  absl::Status interpret();
};
