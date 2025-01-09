#include "ast_printer.h"
#include "cmd_interpreter.h"
#include "sql_interpreter.h"
#include "optional.h"
#include "util.h"
#include "version.h"
#include "scanner.h"
#include "status.h"
#include "sql_parser.h"
#include "parameters.h"
#include "context.h"

#include <cstdlib>
#include <iostream>
#include <string>

bool execute(Context& context, std::vector<std::string>& statements, std::string& statement) {
  std::vector<Token> tokens = Scanner(statement).scan().unwrap();
#ifdef VERDANT_FLAG_DEBUG
//for (auto &token: tokens) {
//  std::cout << token.toString() << std::endl;
//}
#endif

  if (tokens.size() == 0) {
    return true;
  }

  if (tokens[0].type == Token::TOKEN_BACK_SLASH) {
    auto status = CommandInterpreter(context, tokens).interpret();
    if (status == VerdantStatus::TERMINATED) {
      return false;
    }
    return true;
  }

  Optional<AST> optionalAst = SQLParser(tokens).parse();
  if (!optionalAst.unwrappable()) {
    return true;
  }
  AST ast = optionalAst.unwrap();
  
#ifdef VERDANT_FLAG_DEBUG
  ASTPrinter printer("[DEBUG] ");
  printer.print(ast);
#endif
  SQLInterpreter(ast, context).interpret();

  return true;
}


bool loop(Context& context, std::vector<std::string>& statements) {
  std::string statement;
  std::cout << (context.database.unwrappable() ? context.database.peek() + " " : "") << ">> ";
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
  if (!Utility::createDirectory(DATA_PATH)) {
    std::cout << "[ERROR] Cannot create/access data directory at " << DATA_PATH << std::endl;
    exit(1);
  }

  std::vector<std::string> statements;
  Context context = { Optional<std::string>() };

  while (loop(context, statements)) {}
  return VerdantStatus::SUCCESS;
}
