#include "field.h"
#include "column_info.h"
#include "util.h"
#include <cstring>
#include <string>
#include <utility>

// UNSAFE!!
const Optional<std::pair<const char*, size_t>> Field::serialize(const ColumnInfo& column) const {
  if (!match(column)) {
    return Optional<std::pair<const char*, size_t>>(VerdantStatus::INVALID_TYPE);
  }
  switch (column.type) {
    case ColumnInfo::VARCHAR: {
      return std::make_pair(this->value.c_str(), this->value.size());
    }
    case ColumnInfo::INT: {
      char* buffer = (char*)malloc(sizeof(int));
      int intVal = std::stoi(value);
      std::memcpy(&buffer, (char*)&intVal, sizeof(int));
      return std::make_pair((const char*)buffer, sizeof(int));
    }
    case ColumnInfo::FLOAT: {
      char* buffer = (char*)malloc(sizeof(float));
      int floatVal = std::stof(value);
      std::memcpy(&buffer, (char*)&floatVal, sizeof(float));
      return std::make_pair((const char*)buffer, sizeof(float));
    }
  }
}

const bool Field::match(const ColumnInfo& column) const {
  if (column.type == ColumnInfo::VARCHAR && value.size() > column.varcharSize) {
    return false;
  }

  if (column.type == ColumnInfo::INT && !Utility::isInteger(value)) {
    return false;
  }

  if (column.type == ColumnInfo::FLOAT && !Utility::isFloat(value)) {
    return false;
  }

  return true;
}
