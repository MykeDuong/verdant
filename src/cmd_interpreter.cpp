#include "cmd_interpreter.hpp"
#include "absl/status/status.h"
#include "util.hpp"

CommandInterpreter::CommandInterpreter(Context &context,
                                       const std::vector<Token> &tokens)
    : context(context), tokens(tokens), ptr(0) {}

absl::StatusOr<const Token *>
CommandInterpreter::consume(Token::TokenType type, const std::string &message) {
  if (ptr >= tokens.size() || tokens[ptr].type != type) {
    error(message);
    return absl::InvalidArgumentError("Incorrect syntax");
  }
  return eat();
}
const Token *const CommandInterpreter::eat() { return &tokens[this->ptr++]; }

const Token *const CommandInterpreter::current() const {
  return &this->tokens[this->ptr];
}

absl::StatusOr<const Token *> CommandInterpreter::peek() const {
  if (this->ptr + 1 >= tokens.size()) {
    return absl::InvalidArgumentError("Incorrect syntax");
  }
  return &tokens[this->ptr];
}

bool CommandInterpreter::isAtEnd() const {
  return this->ptr == this->tokens.size();
}

void CommandInterpreter::error(const std::string &message) const {
  std::cerr << "[ERROR] Line " << current()->line << ": " << message
            << std::endl;
}

absl::Status CommandInterpreter::interpret() {
  if (!consume(Token::TOKEN_BACK_SLASH, "Expect '/' for commands").ok()) {
    return absl::InvalidArgumentError("Incorrect syntax");
  }

  
  absl::StatusOr<const Token *> optionalCmd = consume(Token::TOKEN_IDENTIFIER, "Invalid token '" + current()->value + "'");

  if (!optionalCmd.ok()) {
    return absl::InvalidArgumentError("Incorrect syntax");
  }

  const Token *cmd = optionalCmd.value();

  if (cmd->value == "q") {
    return absl::CancelledError();
  }
  if (cmd->value == "c") {
    absl::StatusOr<const Token *> optionalDatabaseIdentifier = consume(Token::TOKEN_IDENTIFIER, "Expect database identifier");
    if (!optionalDatabaseIdentifier.ok()) {
    return absl::InvalidArgumentError("Incorrect syntax");
    }
    if (!this->isAtEnd()) {
      error("Unexpected token '" + current()->value + "'");
      return absl::InvalidArgumentError("Incorrect syntax");
    }
    std::string databaseIdentifier = optionalDatabaseIdentifier.value()->value;
    if (!Utility::isFileExist(Utility::getDatabasePath(databaseIdentifier))) {
      return absl::PermissionDeniedError("Database not exist, or Verdant does not have permission to access the database");
    }

    context.database = std::move(databaseIdentifier);
    std::cout << "Connected to database " << context.database.value() << std::endl;

    return absl::OkStatus();
  }

  std::cerr << "Invalid command: " << current()->value << std::endl;
  return absl::InvalidArgumentError("Incorrect syntax");
}
