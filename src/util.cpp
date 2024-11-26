#include "util.h"
#include <cctype>
#include <string>

namespace Utility {
  std::string toLower(const std::string& str) {
    std::string lowerString;

    for (char c: str) {
      lowerString.push_back(std::tolower(c));
    }
    return lowerString;
  }
}
