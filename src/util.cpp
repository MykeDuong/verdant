#include "util.h"
#include <cctype>
#include <string>
#include <sys/stat.h>
#include <iostream>

static std::vector<size_t> findAllChar(const std::string& text, char ch) {
  std::vector<size_t> result;

  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == ch) {
      result.push_back(i);
    }
  }
  return result;
}

namespace Utility {
  std::string toLower(const std::string& str) {
    std::string lowerString;

    for (char c: str) {
      lowerString.push_back(std::tolower(c));
    }
    return lowerString;
  }

  bool createAbsoluteDirectory(const std::string& absolutePath) {
    std::string path = absolutePath;
    if (path[path.size() - 1] != '/') {
      path.push_back('/');
    }
    std::vector<size_t> slashes = findAllChar(path, '/');
    int status;
    for (size_t i = 0; i < slashes.size(); i++) {
      if (i == 0) {
        continue;
      }
      size_t slashIdx = slashes[i];
      std::string curPath = path.substr(0, slashIdx + 1);
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[DEBUG] Creating path " << curPath << std::endl;
#endif
      status = mkdir(curPath.c_str(), 0777);
      if ((status < 0) && (errno != EEXIST)) {
        return false;
      }
    }
    return true;
  }
}
