#pragma once

#include "verdant_object.h"
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
};

class TableNode final: public VerdantObject {
private:
  const std::string name;

public:
  TableNode(const std::string& name);
  std::unordered_map<std::string, ColumnInfo> columns;

  const std::string& getName() const;
  void accept(Visitor* visitor);
  void addColumn(const std::string& name, ColumnInfo&& column);
};
