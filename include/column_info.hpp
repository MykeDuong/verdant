#pragma once

#include <string>
#include <unordered_map>

struct ColumnInfo {
  typedef enum {
    INT, 
    FLOAT,
    VARCHAR,
  } ColumnType;

  ColumnType type;
  size_t varcharSize = 0;
  bool isPrimary = false;

  bool operator==(const ColumnInfo& rhs) const;
  size_t getSize() const;
  std::string toString();
};

typedef std::unordered_map<std::string, std::pair<size_t, ColumnInfo>> Columns;
