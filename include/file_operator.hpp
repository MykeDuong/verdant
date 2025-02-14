#pragma once

#include "optional.hpp"
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

class FileOperator {
private:
  static std::unordered_map<std::string, FileOperator> operatorMapping;
  static std::shared_mutex mappingMutex;
  static void createFileOperator(const std::string &filePath);

  std::fstream file;
  std::shared_mutex fileMutex;
  FileOperator(const std::string &filePath);

public:
  static FileOperator &getFileOperator(const std::string &filePath);

  bool writeNewPage(char *memory);

  bool writeSmallChange(size_t pageIndex, size_t position, char *buffer,
                        size_t bufferSize);

  Optional<std::unique_ptr<char[]>> readPage(size_t index);
};
