#include "optional.h"
#include "version.h"
#include "scanner.h"
#include "status.h"
#include "parser.h"
#include <iostream>
#include <string>

bool loop(Scanner& scanner, Parser& parser, std::string& statement) {
  std::cout << ">> ";
  if (!std::getline(std::cin, statement)) {
    return false;
  }

  if (statement == "\\q" or statement == "exit") {
    return false;
  }

  std::vector<Token> tokens = scanner.scan(statement).unwrap();
  Optional<AST> ast = parser.parse(tokens);
  
  std:: cout << "["; 
  for (auto token: tokens) {
    if (token.value != (*tokens.begin()).value) {
      std::cout << ", ";
    }
    std::cout << '"' << token << '"';
  }
  std:: cout << "]" << std::endl; 
  return true;
}

int main() {
  std::cout << "Verdant" << " Version " << VERDANT_VERSION_MAJOR << "."
            << VERDANT_VERSION_MINOR << std::endl;

#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[Warning] Debug mode enabled." << std::endl;
#endif 

  Scanner scanner;
  Parser parser;
  std::string statement;

  while (loop(scanner, parser, statement)) {}
  return VerdantStatus::SUCCESS;
}