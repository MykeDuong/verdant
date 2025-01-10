#pragma once

#include "column_info.h"
#include "optional.h"
#include "util.h"

struct Field {
  const std::string name;
  const std::string value;
  Utility::BufferUniquePtr<char> buffer = nullptr;

  Field(const std::string &name, const std::string &value);
  const Optional<std::pair<const char*, size_t>> serialize(const ColumnInfo& column);
  const bool match(const ColumnInfo& column) const;
};
