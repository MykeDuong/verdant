#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ast_printer.hpp"
#include "cmd_interpreter.hpp"
#include "context.hpp"
#include "parameters.hpp"
#include "scanner.hpp"
#include "sql_interpreter.hpp"
#include "sql_parser.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

bool execute(Context &context, std::vector<std::string> &statements,
             std::string &statement) {
  context.statement = &statement;
  std::vector<Token> tokens = std::move(Scanner(statement).scan().value());
#ifdef VERDANT_FLAG_DEBUG
// for (auto &token: tokens) {
//   std::cout << token.toString() << std::endl;
// }
#endif

  if (tokens.size() == 0) {
    return true;
  }

  if (tokens[0].type == Token::TOKEN_BACK_SLASH) {
    absl::Status status = CommandInterpreter(context, tokens).interpret();
    if (absl::IsCancelled(status)) {
      return false;
    }
    return true;
  }

  absl::StatusOr<AST> optionalAst = SQLParser(tokens).parse();
  if (!optionalAst.ok()) {
    return true;
  }
  AST ast = std::move(optionalAst.value());

#ifdef VERDANT_FLAG_DEBUG
  ASTPrinter printer("[DEBUG] ");
  printer.print(ast);
#endif
  absl::Status interpretStatus = SQLInterpreter(ast, context).interpret();

  return true;
}

bool loop(Context &context, std::vector<std::string> &statements) {
  std::string statement;
  std::cout << (context.database.ok() ? context.database.value() + " " : "")
            << ">> ";

  if (!std::getline(std::cin, statement)) {
    return false;
  }

  if (statement == "exit") {
    return false;
  }

  bool result = execute(context, statements, statement);
  statements.push_back(std::move(statement));
  return result;
}

int main() {
  std::cout << "Verdant" << " Version " << VERDANT_VERSION_MAJOR << "."
            << VERDANT_VERSION_MINOR << std::endl;

#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[WARNING] Debug mode enabled." << std::endl;
#endif
  if (!Utility::createDirectory(Parameter::DATA_PATH)) {
    std::cout << "[ERROR] Cannot create/access data directory at "
              << Parameter::DATA_PATH << std::endl;
    exit(1);
  }

  std::vector<std::string> statements;
  Context context = {absl::StatusOr<std::string>(), absl::StatusOr<std::string *>()};

  while (loop(context, statements)) {
  }

  return 0;
}
