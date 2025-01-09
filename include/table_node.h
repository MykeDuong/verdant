#pragma once

#include "verdant_object.h"
#include "column_info.h"
#include <vector>

class TableNode final: public VerdantObject {
private:
  const std::string name;

public:
  TableNode(const std::string& name);
  std::vector<ColumnInfo> columns;

  const std::string& getName() const;
  void accept(Visitor* visitor);
  void addColumn(ColumnInfo&& column);
};
