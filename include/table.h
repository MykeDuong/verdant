#pragma once

#include "column_info.h"
#include "storage_interface.h"
#include "parameters.h"
#include "optional.h"
#include "field.h"
#include "util.h"

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

typedef std::pair<const char*, size_t> BinaryRecord;
typedef std::pair<Utility::bufferUniquePtr<char>, size_t> Buffer;
typedef Optional<Buffer> OptionalBuffer;
typedef std::pair<size_t, size_t> Location;

struct TableBlock final: public StorageInterface {
  char block[BLOCK_SIZE];
  std::fstream& file;
  const size_t index;

  TableBlock(std::fstream &file, size_t index);
  std::vector<std::pair<size_t, Buffer>> changes;

  size_t getNumRecords();
  size_t getNextAddress();
  Optional<size_t> getRecordAddress(size_t index);


  bool isEnoughSpace(Buffer& buffer);
  bool addRecord(Buffer&& buffer);
  Optional<BinaryRecord> getRecord(size_t index);
  void save();
};

class Table final: public StorageInterface {
private:
  size_t numBlocks;
  std::fstream file;
  std::unordered_map<size_t, std::unique_ptr<TableBlock>> loadedBlocks;
  std::unordered_map<std::string, std::pair<size_t, ColumnInfo>> columns;

  Optional<TableBlock*> getBlock(size_t index);
  OptionalBuffer createBuffer(const std::vector<Field>& fields);
  bool addRecordToField (const std::vector<Field>& fields, Location location);

public:
  Table(const std::string& name);
  ~Table();
  void save();
  bool addRecord(const std::vector<Field>& fields);
};
