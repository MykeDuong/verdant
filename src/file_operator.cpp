#include "file_operator.hpp"
#include "absl/status/status.h"
#include "parameters.hpp"
#include "util.hpp"

#include <cstdio>
#include <mutex>
#include <utility>

static size_t getBlockCount(std::fstream &file) {
  std::streampos currentPosition = file.tellg();
  file.seekg(0, std::ios::end);
  std::streampos endPosition = file.tellg();
  file.seekg(currentPosition);
  return static_cast<size_t>(endPosition) / Parameter::BLOCK_SIZE;
}

std::unordered_map<std::string, FileOperator> FileOperator::operatorMapping;
std::shared_mutex FileOperator::mappingMutex;

void FileOperator::createFileOperator(const std::string &filePath) {
  std::unique_lock<std::shared_mutex> lock(mappingMutex);
  if (FileOperator::operatorMapping.find(filePath) == operatorMapping.end()) {
    operatorMapping.emplace(
        std::piecewise_construct, std::make_tuple(filePath),
        std::make_tuple(PrivateConstructorStruct{}, filePath));
  }
}

FileOperator::FileOperator(PrivateConstructorStruct,
                           const std::string &filePath)
    : FileOperator::FileOperator(filePath) {}

FileOperator::FileOperator(const std::string &filePath) {
  const std::string expandedFilePath = Utility::expandUser(filePath);
  file.open(expandedFilePath, std::ios::in | std::ios::out | std::ios::binary);
  std::cout << expandedFilePath << std::endl;

  if (!file.is_open()) {
    // Create a new file
    file.open(expandedFilePath, std::ios::out | std::ios::binary);
    file.close();
    file.open(expandedFilePath,
              std::ios::in | std::ios::out | std::ios::binary);
  }
  this->blockCount = getBlockCount(this->file);
}

FileOperator &FileOperator::getFileOperator(const std::string &filePath) {
  std::shared_lock<std::shared_mutex> lock(mappingMutex);
  auto mappingItr = FileOperator::operatorMapping.find(filePath);
  if (mappingItr == FileOperator::operatorMapping.end()) {
    lock.unlock();
    createFileOperator(filePath);
    lock.lock();
  }
  mappingItr = FileOperator::operatorMapping.find(filePath);
  return mappingItr->second;
}

absl::Status FileOperator::deleteFile(const std::string &filePath) {
  const std::string path = Utility::expandUser(filePath);
  int status = remove(path.c_str());
  if (status != 0) {
    return absl::PermissionDeniedError("Cannot delete the specified file");
  }
  return absl::OkStatus();
}

bool FileOperator::deletePage(std::size_t pageIndex) {
  file.seekg(pageIndex * Parameter::BLOCK_SIZE);
  char status = 0;
  file.write(&status, sizeof(char));
  return true;
}

absl::StatusOr<std::size_t> FileOperator::writeNewPage(char *memory) {
  std::unique_lock<std::shared_mutex> lock(this->fileMutex);
  file.seekp(0, std::ios::end);
  file.write(memory, Parameter::BLOCK_SIZE);
  return this->blockCount++;
}

bool FileOperator::writeSmallChange(size_t pageIndex, size_t position,
                                    Buffer buffer) {
  std::unique_lock<std::shared_mutex> lock(this->fileMutex);
  return false;
}

absl::StatusOr<std::unique_ptr<char[]>> FileOperator::readPage(size_t index) {
  std::shared_lock<std::shared_mutex> lock(this->fileMutex);
  file.seekg(index * Parameter::BLOCK_SIZE);
  size_t blockCount = getBlockCount(file);
  if (index >= blockCount) {
    return absl::OutOfRangeError("Block out of range");
  }
  std::unique_ptr<char[]> blockData((char *)malloc(Parameter::BLOCK_SIZE));
  file.read(blockData.get(), Parameter::BLOCK_SIZE);

  return blockData;
}
