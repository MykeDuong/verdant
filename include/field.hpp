#pragma once

#include "column_info.hpp"
#include "optional.hpp"

#include <memory>

struct Field {
  const std::string name;
  const std::string value;
  std::unique_ptr<char[]> buffer = nullptr;

  Field(const std::string &name, const std::string &value);

  const Optional<std::pair<const char *, size_t>>
  serialize(const ColumnInfo &column);

  const bool match(const ColumnInfo &column) const;
  Optional<size_t> getValueSize(const ColumnInfo &column) const;
};
