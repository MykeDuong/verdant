#pragma once

#include <memory>
#include <string>
#include "absl/status/status.h"

namespace Utility {
struct DeleteByFree {
  void operator()(void *ptr) const;
};
template <class T> using BufferUniquePtr = std::unique_ptr<T, DeleteByFree>;

std::string toLower(const std::string &str);
std::string expandUser(const std::string &path);
bool createDirectory(const std::string &path);
bool isDirectoryExist(const std::string &path);
bool isFileExist(const std::string &path);
bool isAlpha(const char c);
bool isFloat(const std::string &str);
bool isInteger(const std::string &str);
std::string getDatabasePath(const std::string &database);
int handleFatalStatus(absl::Status status);
} // namespace Utility
