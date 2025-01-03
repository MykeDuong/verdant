#include "ast_printer.h"
#include "interpreter.h"
#include "optional.h"
#include "util.h"
#include "version.h"
#include "scanner.h"
#include "status.h"
#include "parser.h"
#include "parameters.h"

#include <cstdlib>
#include <iostream>
#include <string>

bool loop(Scanner& scanner, std::string& statement) {
  std::cout << ">> ";
  if (!std::getline(std::cin, statement)) {
    return false;
  }

  if (statement == "\\q" or statement == "exit") {
    return false;
  }

  std::vector<Token> tokens = scanner.scan(statement).unwrap();
  Optional<AST> astOrError = Parser(tokens).parse();
  if (astOrError.error != 0) {
    return true;
  }
  AST ast = astOrError.unwrap();
  
#ifdef VERDANT_FLAG_DEBUG
  ASTPrinter printer("[DEBUG] ");
  printer.print(ast);
#endif
  Interpreter().interpret(std::move(ast));

  return true;
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

  Scanner scanner;
  std::string statement;

  while (loop(scanner, statement)) {}
  return VerdantStatus::SUCCESS;
}
