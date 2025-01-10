#include "field.h"
#include "column_info.h"
#include "util.h"
#include <cstring>
#include <string>
#include <utility>

// UNSAFE!!
const Optional<std::pair<const char *, size_t>>
Field::serialize(const ColumnInfo &column) {
  if (!match(column)) {
    return Optional<std::pair<const char *, size_t>>(
        VerdantStatus::INVALID_TYPE);
  }
  switch (column.type) {
  case ColumnInfo::VARCHAR: {
    return std::make_pair(this->value.c_str(), this->value.size());
  }
  case ColumnInfo::INT: {
    if (buffer == nullptr) {
      buffer.reset((char *)malloc(sizeof(int)));
      int intVal = std::stoi(value);
      std::memcpy(buffer.get(), (char *)&intVal, sizeof(int));
    }
    return std::make_pair((const char *)buffer.get(), sizeof(int));
  }
  case ColumnInfo::FLOAT: {
    if (buffer == nullptr) {
      buffer.reset((char *)malloc(sizeof(float)));
      float floatVal = std::stof(value);
      std::memcpy(buffer.get(), (char *)&floatVal, sizeof(float));
    }
    return std::make_pair((const char *)buffer.get(), sizeof(float));
  }
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cerr << "[ERROR] Unreachable" << std::endl;
#endif
  VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  exit(VerdantStatus::INTERNAL_ERROR);
}

const bool Field::match(const ColumnInfo &column) const {
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
