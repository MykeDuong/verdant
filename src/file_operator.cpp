#include "file_operator.hpp"
#include "parameters.hpp"
#include "status.hpp"

#include <mutex>

static size_t getBlockCount(std::fstream &file) {
  std::streampos currentPosition = file.tellg();
  file.seekg(0, std::ios::end);
  std::streampos endPosition = file.tellg();
  file.seekg(currentPosition);
  return static_cast<size_t>(endPosition) / BLOCK_SIZE;
}

std::unordered_map<std::string, FileOperator> FileOperator::operatorMapping;
std::shared_mutex FileOperator::mappingMutex;

void FileOperator::createFileOperator(const std::string &filePath) {
  std::unique_lock<std::shared_mutex> lock(mappingMutex);
  if (operatorMapping.find(filePath) == operatorMapping.end()) {
    operatorMapping.emplace(filePath, FileOperator(filePath));
  }
}

FileOperator::FileOperator(const std::string &filePath) {
  file.open(filePath, std::ios::in | std::ios::out | std::ios::binary);

  if (!file.is_open()) {
    // Create a new file
    file.open(filePath, std::ios::out | std::ios::binary);
    file.close();
    file.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
  }
}

FileOperator &FileOperator::getFileOperator(const std::string &filePath) {
  std::shared_lock<std::shared_mutex> lock(mappingMutex);
  auto mappingItr = FileOperator::operatorMapping.find(filePath);
  if (mappingItr == FileOperator::operatorMapping.end()) {
    lock.unlock();
    createFileOperator(filePath);
    lock.lock();
  }
  return mappingItr->second;
}

bool FileOperator::writeNewPage(char *memory) {
  std::unique_lock<std::shared_mutex> lock(this->fileMutex);
  file.seekg(0, std::ios::end);
  file.write(memory, BLOCK_SIZE);
  return false;
}

bool FileOperator::writeSmallChange(size_t pageIndex, size_t position,
                                    char *buffer, size_t bufferSize) {
  std::unique_lock<std::shared_mutex> lock(this->fileMutex);
  return false;
}

Optional<std::unique_ptr<char[]>> FileOperator::readPage(size_t index) {
  std::shared_lock<std::shared_mutex> lock(this->fileMutex);
  file.seekg(index * BLOCK_SIZE);
  size_t blockCount = getBlockCount(file);
  if (index >= blockCount) {
    return VerdantStatus::OUT_OF_BOUND;
  }
  std::unique_ptr<char[]> buffer((char *)malloc(BLOCK_SIZE));
  file.read(buffer.get(), BLOCK_SIZE);

  return buffer;
}
