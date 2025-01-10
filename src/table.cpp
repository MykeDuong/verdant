#include "table.h"
#include "field.h"
#include "parameters.h"
#include "status.h"
#include "util.h"
#include <cstring>
#include <ios>
#include <iosfwd>
#include <tuple>
#include <utility>

static size_t getBlockCount(std::fstream &file) {
  std::streampos currentPosition = file.tellg();
  file.seekg(0, std::ios::end);
  std::streampos endPosition = file.tellg();
  file.seekg(currentPosition);
  return static_cast<size_t>(endPosition) / BLOCK_SIZE;
}

TableBlock::TableBlock(std::fstream &file, size_t index) : file(file), index(index) {
  size_t blockCount = getBlockCount(file);
  if (blockCount > index) {
#ifdef VERDANT_FLAG_DEBUG
    std::cerr << "[ERROR] Trying to create a block out of bound" << std::endl;
#endif
    VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  }
  if (index == blockCount) {
    std::memset(&this->block, 0, BLOCK_SIZE);
  }
  file.seekg(index * BLOCK_SIZE);
  file.read(reinterpret_cast<char*>(&this->block), BLOCK_SIZE);
}

size_t TableBlock::getNumRecords() {
  size_t num;
  std::memcpy(&num, &block[BLOCK_SIZE - sizeof(size_t)], sizeof(size_t));
  return num;
}

size_t TableBlock::getNextAddress() {
  size_t nextAddress;
  std::memcpy(&nextAddress, &block[BLOCK_SIZE - 2 * sizeof(size_t)], sizeof(size_t));
  return nextAddress;
}

Optional<size_t> TableBlock::getRecordAddress(size_t index) {
  size_t numRecords = getNumRecords();
  if (index > numRecords) {
    return Optional<size_t>();
  }
  if (index == numRecords) {
    return getNextAddress();
  }
  size_t address;
  std::memcpy(&address, &block[BLOCK_SIZE - (2 + index + 1) * sizeof(size_t)], sizeof(size_t));
  return address;
}

bool TableBlock::isEnoughSpace(Buffer& buffer) {
  size_t nextAddress = getNextAddress();
  size_t numRecords = getNumRecords();
  size_t startOfRecordPointers = BLOCK_SIZE - (2 + numRecords) * sizeof(size_t);
  if (startOfRecordPointers - nextAddress < sizeof(size_t) + buffer.second) {
    return false;
  }
  return true;
}

bool TableBlock::addRecord(Buffer&& buffer) {
  if (!isEnoughSpace(buffer)) {
    return false;
  }
  size_t nextAddress = getNextAddress();
  changes.push_back(std::make_pair(nextAddress, std::move(buffer)));
  return true;
}

Optional<BinaryRecord> TableBlock::getRecord(size_t index) {
  size_t numRecords = getNumRecords();
  if (index >= numRecords) {
    return Optional<BinaryRecord>();
  }
  const char* record = &block[getRecordAddress(index).unwrap()];
  size_t size = getRecordAddress(index + 1).unwrap() - getRecordAddress(index).unwrap();

  return Optional<BinaryRecord>(std::make_pair(record, size));
}

void TableBlock::save() {
  for (auto& change: changes) {
    size_t position = change.first;
    Buffer& buffer = change.second;
    char* changePtr = buffer.first.get();
    size_t changeSize = buffer.second;
    std::memcpy(&this->block[position], changePtr, changeSize);
    file.seekg(index * BLOCK_SIZE + position);
    file.write(changePtr, changeSize);
    changes.clear();
  }
}

Table::Table(Context& context, const std::string& name, Columns&& columns) : columns(std::move(columns)), context(context) {
  std::string absolutePath = Utility::getDatabasePath(context.database.unwrap()) + name + "/";
  file.open(absolutePath, std::ios::in | std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    // Create a new file
    file.open(absolutePath, std::ios::out | std::ios::binary);
    file.close();
    file.open(absolutePath, std::ios::in | std::ios::out | std::ios::binary);
  }
}

Table::~Table() {
  file.close();
}

void Table::save() {
  for (auto& idxAndBlock: loadedBlocks) {
    idxAndBlock.second->save();
  }
}

Optional<TableBlock*> Table::getBlock(size_t index) {
  size_t blockCount = getBlockCount(file);
  if (index > blockCount) {
    return Optional<TableBlock*>(VerdantStatus::OUT_OF_BOUND);
  }
  if (loadedBlocks.find(index) == loadedBlocks.end()) {
    loadedBlocks[index] = std::unique_ptr<TableBlock>(new TableBlock(file, index));
  }
  return loadedBlocks[index].get();
}

bool Table::addRecord(const std::vector<Field>& fields) {
  size_t blockCount = getBlockCount(file);
  OptionalBuffer optionalBuffer = createBuffer(fields);
  if (!optionalBuffer.unwrappable()) {
    return false;
  }
  Buffer buffer = optionalBuffer.unwrap();

  TableBlock* lastBlock = getBlock(blockCount - 1).unwrap();

  if (lastBlock->isEnoughSpace(buffer)) {
    lastBlock->addRecord(std::move(buffer));
    return true;
  }

  TableBlock* newBlock = getBlock(blockCount).unwrap();

  newBlock->addRecord(std::move(buffer));
  return true;
}

bool addRecordToField(const std::vector<Field>& fields, Location location) {
  return true;
}


OptionalBuffer Table::createBuffer(const std::vector<Field>& fields) {
  if (fields.size() != columns.size()) {
    std::cerr << "[ERROR] Invalid number of fields provided" << std::endl;
    return OptionalBuffer(VerdantStatus::INVALID_TYPE);
  }

  std::vector<std::pair<const Field*, const ColumnInfo*>> orderedFields;
  orderedFields.resize(fields.size());

  std::vector<bool> filled;
  filled.resize(fields.size());
  for (size_t i = 0; i < filled.size(); i++) {
    filled[i] = false;
  }
  size_t totalSize = 0;
  for (const Field &field: fields) {
    if (columns.find(field.name) == columns.end()) {
      std::cerr << "[ERROR] Field name not found" << std::endl;
      return OptionalBuffer(VerdantStatus::INVALID_TYPE);
    }
    auto& column = columns[field.name];
    size_t index = column.first;
    ColumnInfo &info = column.second;
    if (filled[index] == true) {
      std::cerr << "[ERROR] Duplicate field" << std::endl;
      return OptionalBuffer(VerdantStatus::INVALID_TYPE);
    }
    if (!field.match(info)) {
      std::cerr << "[ERROR] Invalid value for type " << std::endl;
      return OptionalBuffer(VerdantStatus::INVALID_TYPE);
    }
    totalSize += sizeof(size_t) + info.getSize();
    filled[index] = true;
    orderedFields[index] = std::make_pair(&field, &info);
  }

  Utility::BufferUniquePtr<char> buffer((char*)malloc(totalSize));
  size_t ptr = 0;
  for (auto &pair: orderedFields) {
    const Field* field;
    const ColumnInfo* info;
    std::tie(field, info) = pair;
    auto optionalSerialization = field->serialize(*info);
    if (!optionalSerialization.unwrappable()) {
#ifdef VERDANT_FLAG_DEBUG
      std::cerr << "[ERROR] Unreachable" << std::endl;
      VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
#endif
    }
    const char* serializedValue;
    size_t valueSize; 
    std::tie(serializedValue, valueSize) = optionalSerialization.unwrap();
    size_t fieldSize = info->getSize();
    std::memcpy(&buffer.get()[ptr], &fieldSize, sizeof(size_t));
    ptr += sizeof(size_t);
    std::memcpy(&buffer.get()[ptr], &info->type, sizeof(ColumnInfo::ColumnType));
    ptr += sizeof(ColumnInfo::ColumnType);
    switch (info->type) {
      case ColumnInfo::INT: 
      case ColumnInfo::FLOAT: 
        std::memcpy(&buffer.get()[ptr], serializedValue, valueSize);
        ptr += valueSize;
        break;
      case ColumnInfo::VARCHAR: 
        std::memcpy(&buffer.get()[ptr], &valueSize, sizeof(size_t));
        ptr += sizeof(size_t);
        std::memcpy(&buffer.get()[ptr], serializedValue, valueSize);
        ptr += valueSize;
        break;
    }
  }
  return std::make_pair(std::move(buffer), totalSize);
}
