#pragma once

#include <string>
#include <memory>

namespace Utility {
  struct DeleteByFree {
    void operator()(void* ptr) const;
  };
  template<class T> using BufferUniquePtr = std::unique_ptr<T, DeleteByFree>;
  
  std::string toLower(const std::string& str);
  bool createDirectory(const std::string& path);
  bool isDirectoryExist(const std::string& path);
  bool isAlpha(const char c);
  bool isFloat(const std::string& str);
  bool isInteger(const std::string& str);
  std::string getDatabasePath(const std::string& database);
}

