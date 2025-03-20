#pragma once

#include "absl/status/statusor.h"
#include "buffer.hpp"

#include <fstream>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

class FileOperator {
private:
  struct PrivateConstructorStruct {};
  static std::unordered_map<std::string, FileOperator> operatorMapping;
  static std::shared_mutex mappingMutex;
  static void createFileOperator(const std::string &filePath);
  static FileOperator create(const std::string &filePath);

  std::fstream file;
  std::shared_mutex fileMutex;
  FileOperator(const std::string &filePath);
  std::size_t blockCount;

public:
  FileOperator(PrivateConstructorStruct, const std::string &filePath);

  static FileOperator &getFileOperator(const std::string &filePath);

  absl::StatusOr<std::size_t> writeNewPage(char *memory);

  bool writeSmallChange(size_t pageIndex, size_t position, Buffer buffer);

  bool deletePage(std::size_t pageIndex);

  absl::StatusOr<std::unique_ptr<char[]>> readPage(size_t index);
};
