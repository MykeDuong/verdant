#include "optional.h"
#include "version.h"
#include "scanner.h"
#include <status.h>
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

  std::vector<std::string> tokens = scanner.parse(statement).unwrap();
  
  std:: cout << "["; 
  for (auto token: tokens) {
    if (token != *tokens.begin()) {
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
  std::string statement;

  while (loop(scanner, statement)) {}
  return VerdantStatus::SUCCESS;
}
