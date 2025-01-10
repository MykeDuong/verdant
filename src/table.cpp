#include "table.h"
#include "field.h"
#include "parameters.h"
#include "status.h"
#include "util.h"
#include "verdant_object.h"
#include <cstring>
#include <ios>
#include <iosfwd>
#include <tuple>
#include <utility>

static bool openFile(std::fstream &file, const std::string &path) {
  file.open(path, std::ios::in | std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    // Create a new file
    file.open(path, std::ios::out | std::ios::binary);
    file.close();
    file.open(path, std::ios::in | std::ios::out | std::ios::binary);
  }
  return file.is_open();
}

static size_t getBlockCount(std::fstream &file) {
  std::streampos currentPosition = file.tellg();
  file.seekg(0, std::ios::end);
  std::streampos endPosition = file.tellg();
  file.seekg(currentPosition);
  return static_cast<size_t>(endPosition) / BLOCK_SIZE;
}

TableBlock::TableBlock(std::fstream &file, size_t index)
    : file(file), index(index) {
  size_t blockCount = getBlockCount(file);
  if (blockCount > index) {
#ifdef VERDANT_FLAG_DEBUG
    std::cerr << "[ERROR] Trying to create a block out of bound" << std::endl;
#endif
    VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  }
  if (index == blockCount) {
    std::memset(&this->block, 0, BLOCK_SIZE);
    recordCount = 0;
    nextAddress = 0;
  } else {
    file.seekg(index * BLOCK_SIZE);
    file.read(reinterpret_cast<char *>(&this->block), BLOCK_SIZE);
    recordCount = getRecordCountOnFile();
    nextAddress = getNextAddressOnFile();
  }
}

size_t TableBlock::getRecordCountOnFile() {
  size_t num;
  std::memcpy(&num, &block[BLOCK_SIZE - sizeof(size_t)], sizeof(size_t));
  return num;
}

size_t TableBlock::getNextAddressOnFile() {
  size_t nextAddress;
  std::memcpy(&nextAddress, &block[BLOCK_SIZE - 2 * sizeof(size_t)],
              sizeof(size_t));
  return nextAddress;
}

Optional<size_t> TableBlock::getRecordAddress(size_t index) {
  size_t numRecords = getRecordCountOnFile();
  if (index > numRecords) {
    return Optional<size_t>();
  }
  if (index == numRecords) {
    return getNextAddressOnFile();
  }
  size_t address;
  std::memcpy(&address, &block[BLOCK_SIZE - (2 + index + 1) * sizeof(size_t)],
              sizeof(size_t));
  return address;
}

bool TableBlock::isEnoughSpace(Buffer &buffer) {
  size_t bookkeepLocation = getBookkeepLocation();
  return bookkeepLocation - nextAddress >= sizeof(size_t) + buffer.second;
}

bool TableBlock::addRecord(Buffer &&buffer) {
  if (!isEnoughSpace(buffer)) {
    return false;
  }
  size_t size = buffer.second;
  changes.push_back(std::make_tuple(recordCount++, nextAddress, std::move(buffer)));
  nextAddress += size;
  return true;
}

size_t TableBlock::getBookkeepLocation() {
  return BLOCK_SIZE - 2 * sizeof(size_t) - recordCount * sizeof(size_t);
}

Optional<BinaryRecord> TableBlock::getRecord(size_t index) {
  size_t numRecords = getRecordCountOnFile();
  if (index >= numRecords) {
    return Optional<BinaryRecord>();
  }
  const char *record = &block[getRecordAddress(index).unwrap()];
  size_t size =
      getRecordAddress(index + 1).unwrap() - getRecordAddress(index).unwrap();

  return Optional<BinaryRecord>(std::make_pair(record, size));
}

void TableBlock::saveRecordPointer(size_t index, size_t pointer) {
  std::memcpy(&this->block[BLOCK_SIZE - 2 * sizeof(size_t) - (index + 1) * sizeof(size_t)], reinterpret_cast<char*>(&pointer), sizeof(size_t));
  file.seekg((index + 1) * BLOCK_SIZE - 2 * sizeof(size_t) - (index + 1) * sizeof(size_t));
  file.write(reinterpret_cast<char*>(&pointer), sizeof(size_t));
}

void TableBlock::save() {
  if (getBlockCount(file) == this->index) {
    file.seekg(index * BLOCK_SIZE);
    file.write(this->block, 8192);
  }

  for (auto &change : changes) {
    size_t index = std::get<0>(change);
    size_t position = std::get<1>(change);
    Buffer &buffer = std::get<2>(change);
    char *changePtr = buffer.first.get();
    size_t changeSize = buffer.second;
    std::memcpy(&this->block[position], changePtr, changeSize);
    file.seekg(index * BLOCK_SIZE + position);
    file.write(changePtr, changeSize);
  }
  for (auto& change: changes) {
    this->saveRecordPointer(std::get<0>(change), std::get<1>(change));
  }
  changes.clear();
  // Write recordCount
  std::memcpy(&this->block[BLOCK_SIZE - sizeof(size_t)], reinterpret_cast<char*>(&recordCount), sizeof(size_t));

  file.seekg((index + 1) * (BLOCK_SIZE) - sizeof(size_t));
  file.write(reinterpret_cast<char*>(&recordCount), sizeof(size_t));

  // Write next pointer
  std::memcpy(&this->block[BLOCK_SIZE - 2 * sizeof(size_t)], reinterpret_cast<char*>(&nextAddress), sizeof(size_t));
  file.seekg((index + 1) * BLOCK_SIZE - 2 * sizeof(size_t));
  file.write(reinterpret_cast<char*>(&nextAddress), sizeof(size_t));
}

Table::Table(Context *context, const std::string &name, Columns &&columns)
    : columns(std::move(columns)), context(context) {
  std::string absolutePath =
      Utility::getDatabasePath(context->database.unwrap()) + name;
  if (openFile(file, absolutePath)) {
    std::cerr << "[ERROR] Cannot create the master table" << std::endl;
    VerdantStatus::handleError(VerdantStatus::INVALID_PERMISSION);
  }
}

std::unique_ptr<Table> Table::createMasterTable(const std::string &database) {
  Columns columns;
  columns["name"] = {0, {ColumnInfo::VARCHAR, MAX_OBJECT_NAME, true}};
  columns["type"] = {1, {ColumnInfo::INT, 0, false}};

  std::string masterTableName = database + "_verdant_master.vtbl";
  std::unique_ptr<Table> verdantMaster(new Table(database, masterTableName, std::move(columns)));
  std::vector<Field> record;
  record.push_back({"name", masterTableName});
  record.push_back({"type", std::to_string(VerdantObjectType::TABLE)});
  verdantMaster->addRecord(record);

  verdantMaster->save();
  return verdantMaster;
}

std::unique_ptr<Table> Table::getMasterTable(const std::string &database) {
  Columns columns;
  columns["name"] = {0, {ColumnInfo::VARCHAR, MAX_OBJECT_NAME, true}};
  columns["type"] = {1, {ColumnInfo::INT, 0, false}};

  std::string masterTableName = database + "_verdant_master.vtbl";
  std::unique_ptr<Table> verdantMaster(new Table(database, masterTableName, std::move(columns)));

  return verdantMaster;
}

Table::Table(const std::string &database, const std::string &name,
             Columns &&columns)
    : columns(columns),
      context(Optional<Context *>(VerdantStatus::UNSPECIFIED_DATABASE)) {
  std::string absolutePath = Utility::getDatabasePath(database) + name;
  openFile(file, absolutePath);
}

Table::~Table() { file.close(); }

void Table::save() {
  for (auto &idxAndBlock : loadedBlocks) {
    idxAndBlock.second->save();
  }
}

Optional<TableBlock *> Table::getBlock(size_t index) {
  size_t blockCount = getBlockCount(file);
  if (index < 0 || index > blockCount) {
    return Optional<TableBlock *>(VerdantStatus::OUT_OF_BOUND);
  }
  if (loadedBlocks.find(index) == loadedBlocks.end()) {
    loadedBlocks[index] =
        std::unique_ptr<TableBlock>(new TableBlock(file, index));
  }
  return loadedBlocks[index].get();
}

bool Table::addRecord(std::vector<Field> &fields) {
  size_t blockCount = getBlockCount(file);
  OptionalBuffer optionalBuffer = createBuffer(fields);
  if (!optionalBuffer.unwrappable()) {
    return false;
  }
  Buffer buffer = optionalBuffer.unwrap();

  if (blockCount > 0) {
    TableBlock *lastBlock = getBlock(blockCount - 1).unwrap();
    if (lastBlock->isEnoughSpace(buffer)) {
      lastBlock->addRecord(std::move(buffer));
      return true;
    }
  }

  TableBlock *newBlock = getBlock(blockCount).unwrap();

  newBlock->addRecord(std::move(buffer));
  return true;
}

bool addRecordToField(std::vector<Field> &fields, Location location) {
  return false;
}

OptionalBuffer Table::createBuffer(std::vector<Field> &fields) {
  if (fields.size() != columns.size()) {
    std::cerr << "[ERROR] Invalid number of fields provided" << std::endl;
    return OptionalBuffer(VerdantStatus::INVALID_TYPE);
  }

  std::vector<std::pair<Field *, const ColumnInfo *>> orderedFields;
  orderedFields.resize(fields.size());

  std::vector<bool> filled;
  filled.resize(fields.size());
  for (size_t i = 0; i < filled.size(); i++) {
    filled[i] = false;
  }
  size_t totalSize = 0;
  for (Field &field : fields) {
    if (columns.find(field.name) == columns.end()) {
      std::cerr << "[ERROR] Field name not found" << std::endl;
      return OptionalBuffer(VerdantStatus::INVALID_TYPE);
    }
    auto &column = columns[field.name];
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

  Utility::BufferUniquePtr<char> buffer((char *)malloc(totalSize));
  size_t ptr = 0;
  for (auto &pair : orderedFields) {
    Field *field;
    const ColumnInfo *info;
    std::tie(field, info) = pair;
    auto optionalSerialization = field->serialize(*info);
    if (!optionalSerialization.unwrappable()) {
#ifdef VERDANT_FLAG_DEBUG
      std::cerr << "[ERROR] Unreachable" << std::endl;
      VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
#endif
    }
    const char *serializedValue;
    size_t valueSize;
    std::tie(serializedValue, valueSize) = optionalSerialization.unwrap();
    size_t fieldSize = info->getSize();
    std::memcpy(&buffer.get()[ptr], &fieldSize, sizeof(size_t));
    ptr += sizeof(size_t);
    std::memcpy(&buffer.get()[ptr], &info->type,
                sizeof(ColumnInfo::ColumnType));
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
