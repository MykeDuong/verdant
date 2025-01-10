#pragma once

#include "column_info.h"
#include "context.h"
#include "field.h"
#include "optional.h"
#include "parameters.h"
#include "storage_interface.h"
#include "util.h"

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::pair<const char *, size_t> BinaryRecord;
typedef std::pair<Utility::BufferUniquePtr<char>, size_t> Buffer;
typedef Optional<Buffer> OptionalBuffer;
typedef std::pair<size_t, size_t> Location;

struct TableBlock final : public StorageInterface {
  char block[BLOCK_SIZE];
  std::fstream &file;
  const size_t index;
  size_t recordCount;
  size_t nextAddress;

  TableBlock(std::fstream &file, size_t index);
  // Index, address, buffer
  std::vector<std::tuple<size_t, size_t, Buffer>> changes;

  size_t getBookkeepLocation();
  size_t getRecordCountOnFile();
  size_t getNextAddressOnFile();
  Optional<size_t> getRecordAddress(size_t index);

  bool isEnoughSpace(Buffer &buffer);
  bool addRecord(Buffer &&buffer);
  Optional<BinaryRecord> getRecord(size_t index);
  void saveRecordPointer(size_t index, size_t pointer);
  void save();
};

class Table final : public StorageInterface {
private:
  std::fstream file;
  std::unordered_map<size_t, std::unique_ptr<TableBlock>> loadedBlocks;
  Columns columns;
  Optional<Context *> context;

  Optional<TableBlock *> getBlock(size_t index);
  OptionalBuffer createBuffer(std::vector<Field> &fields);
  bool addRecordToField(std::vector<Field> &fields, Location location);

public:
  Table(Context *context, const std::string &name, Columns &&columns);
  Table(const std::string &database, const std::string &name,
        Columns &&columns);
  static std::unique_ptr<Table> createMasterTable(const std::string &database);
  static std::unique_ptr<Table> getMasterTable(const std::string &database);
  ~Table();
  void save();
  bool addRecord(std::vector<Field> &fields);
};
