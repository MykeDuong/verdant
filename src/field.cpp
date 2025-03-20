#include "field.hpp"
#include "absl/status/status.h"
#include "column_info.hpp"
#include "util.hpp"
#include <cstring>
#include <string>

Field::Field(const std::string &name, const std::string &value)
    : name(name), value(value) {}

const absl::StatusOr<Buffer> Field::serialize(const ColumnInfo &column) {
  if (!match(column)) {
    return absl::InvalidArgumentError("Invalid Type");
  }

  switch (column.type) {
  case ColumnInfo::VARCHAR: {
    return Buffer(this->value.c_str(), this->value.size());
  }
  case ColumnInfo::INT: {
    if (serializedData == nullptr) {
      serializedData.reset(new char[sizeof(int)]);
      int intVal = std::stoi(value);
      std::memcpy(serializedData.get(), (char *)&intVal, sizeof(int));
    }
    return Buffer((const char *const)serializedData.get(), sizeof(int));
  }
  case ColumnInfo::FLOAT: {
    if (serializedData == nullptr) {
      serializedData.reset(new char[sizeof(float)]);
      float floatVal = std::stof(value);
      std::memcpy(serializedData.get(), (char *)&floatVal, sizeof(float));
    }
    return Buffer((const char *const)serializedData.get(), sizeof(float));
  }
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cerr << "[ERROR] Unreachable" << std::endl;
#endif
  exit(Utility::handleFatalStatus(absl::InternalError("Unreachable")));
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

absl::StatusOr<size_t> Field::getValueSize(const ColumnInfo &column) const {
  if (column.type == ColumnInfo::VARCHAR && value.size() > column.varcharSize) {
    return absl::InvalidArgumentError("Invalid Type");
  }

  if (column.type == ColumnInfo::INT && !Utility::isInteger(value)) {
    return absl::InvalidArgumentError("Invalid Type");
  }

  if (column.type == ColumnInfo::FLOAT && !Utility::isFloat(value)) {
    return absl::InvalidArgumentError("Invalid Type");
  }

  switch (column.type) {
  case ColumnInfo::VARCHAR:
    return this->value.size();
  case ColumnInfo::INT:
    return sizeof(int);
  case ColumnInfo::FLOAT:
    return sizeof(float);
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[ERROR] Unreachable" << std::endl;
#endif
  exit(Utility::handleFatalStatus(absl::InternalError("Unreachable")));
}
