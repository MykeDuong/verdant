#include "ast_printer.h"
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

size_t BLOCK_SIZE = 8192;

bool loop(Scanner& scanner, Parser& parser, std::string& statement) {
  std::cout << ">> ";
  if (!std::getline(std::cin, statement)) {
    return false;
  }

  if (statement == "\\q" or statement == "exit") {
    return false;
  }

  std::vector<Token> tokens = scanner.scan(statement).unwrap();
  Optional<AST> astOrError = parser.parse(tokens);
  AST ast = astOrError.unwrap();
  
#ifdef VERDANT_FLAG_DEBUG
  ASTPrinter printer("[DEBUG] ");
  printer.print(ast);
#endif

  return true;
}

int main() {
  std::cout << "Verdant" << " Version " << VERDANT_VERSION_MAJOR << "."
            << VERDANT_VERSION_MINOR << std::endl;

#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[WARNING] Debug mode enabled." << std::endl;
#endif 
  std::string dataPath = "/etc/verdant/";
  if (!Utility::createAbsoluteDirectory(dataPath)) {
    std::cout << "[ERROR] Cannot create/access data directory at " << dataPath << std::endl;
    exit(1);
  }

  Scanner scanner;
  Parser parser;
  std::string statement;

  while (loop(scanner, parser, statement)) {}
  return VerdantStatus::SUCCESS;
}
