#pragma once

#include "absl/status/statusor.h"
#include "buffer.hpp"
#include "column_info.hpp"

#include <memory>

struct Field {
  const std::string name;
  const std::string value;
  std::unique_ptr<char[]> serializedData = nullptr;

  Field(const std::string &name, const std::string &value);

  const absl::StatusOr<Buffer> serialize(const ColumnInfo &column);

  const bool match(const ColumnInfo &column) const;
  absl::StatusOr<size_t> getValueSize(const ColumnInfo &column) const;
};
