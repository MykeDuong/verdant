#pragma once

#include "verdant_object.h"
#include "column_info.h"
#include <unordered_map>

class TableNode final: public VerdantObject {
private:
  const std::string name;

public:
  TableNode(const std::string& name);
  std::unordered_map<std::string, std::pair<size_t, ColumnInfo>> columns;

  const std::string& getName() const;
  const VerdantObjectType getType() const;
  void accept(Visitor* visitor);
  bool addColumn(std::string name, size_t position, ColumnInfo&& column);
};
