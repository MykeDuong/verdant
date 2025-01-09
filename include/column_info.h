#pragma once

#include <string>

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
