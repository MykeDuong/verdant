#pragma once

#include <string>
#include <vector>

namespace Utility {
  std::string toLower(const std::string& str);
  bool createDirectory(const std::string& path);
  bool isDirectoryExist(const std::string& path);
  bool isAlpha(const char c);
  std::string getDatabasePath(const std::string& database);
}

