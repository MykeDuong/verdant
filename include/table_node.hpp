#pragma once

#include "column_info.hpp"
#include "verdant_object.hpp"

class TableNode final : public VerdantObject {
private:
  const std::string name;

public:
  TableNode(const std::string &name);
  Columns columns;

  const std::string &getName() const;
  const VerdantObjectType getType() const;
  void accept(Visitor &visitor);
  bool addColumn(std::string name, size_t position, ColumnInfo &&column);
};
