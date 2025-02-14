#include "cmd_interpreter.hpp"
#include "status.hpp"
#include "util.hpp"

CommandInterpreter::CommandInterpreter(Context& context, const std::vector<Token>& tokens) : context(context), tokens(tokens), ptr(0) {}

Optional<const Token*> CommandInterpreter::consume(Token::TokenType type, const std::string& message) {
  if (ptr >= tokens.size() || tokens[ptr].type != type) {
    error(message);
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }
  return eat();
}
const Token* const CommandInterpreter::eat() {
  return &tokens[this->ptr++];
}

const Token* const CommandInterpreter::current() const {
  return &this->tokens[this->ptr];
}

Optional<const Token*> CommandInterpreter::peek() const {
  if (this->ptr + 1 >= tokens.size()) {
    return Optional<const Token*>(VerdantStatus::INVALID_SYNTAX);
  }
  return &tokens[this->ptr];
}

bool CommandInterpreter::isAtEnd() const {
  return this->ptr == this->tokens.size();
}

void CommandInterpreter::error(const std::string& message) const {
  std::cerr << "[ERROR] Line " << current()->line << ": " << message << std::endl;
}

VerdantStatus::StatusEnum CommandInterpreter::interpret() {
  if (!consume(Token::TOKEN_BACK_SLASH, "Expect '/' for commands").unwrappable()) {
    return VerdantStatus::INVALID_SYNTAX;
  }

  auto optionalCmd = consume(Token::TOKEN_IDENTIFIER, "Invalid token '" + current()->value + "'");
  if (!optionalCmd.unwrappable()) {
    return VerdantStatus::INVALID_SYNTAX;
  }
  auto cmd = optionalCmd.unwrap();

  if (cmd->value == "q") {
    return VerdantStatus::TERMINATED;
  } 
  if (cmd->value == "c") {
    auto optionalDatabaseIdentifier = consume(Token::TOKEN_IDENTIFIER, "Expect database identifier");
    if (!optionalDatabaseIdentifier.unwrappable()) {
      return VerdantStatus::INVALID_SYNTAX;
    }
    if (!this->isAtEnd()) {
      error("Unexpected token '" + current()->value + "'");
      return VerdantStatus::INVALID_SYNTAX;
    }
    std::string databaseIdentifier = optionalDatabaseIdentifier.unwrap()->value;
    if (!Utility::isDirectoryExist(Utility::getDatabasePath(databaseIdentifier))) {
      std::cerr << "[ERROR] Database not exist, or Verdant does not have permission to access the database" << std::endl;
      return VerdantStatus::INVALID_PERMISSION;
    }

    context.database.setValue(std::move(databaseIdentifier));
    std::cout << "Connected to database " << context.database.peek() << std::endl;

    return VerdantStatus::SUCCESS;
  }

  std::cerr << "Invalid command: " << current()->value << std::endl;
  return VerdantStatus::INVALID_SYNTAX;
}
