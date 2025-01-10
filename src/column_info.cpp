#include "column_info.h"
#include "status.h"

#include <iostream>

bool ColumnInfo::operator==(const ColumnInfo& rhs) const {
  return type == rhs.type

      && isPrimary == rhs.isPrimary
      && (type == ColumnInfo::VARCHAR ? varcharSize == rhs.varcharSize : true);
}

size_t ColumnInfo::getSize() const {
  switch (type) {
    case ColumnInfo::INT:
      return sizeof(ColumnInfo::ColumnType) + sizeof(int);
    case ColumnInfo::FLOAT:
      return sizeof(ColumnInfo::ColumnType) + sizeof(float);
    case ColumnInfo::VARCHAR:
      return sizeof(ColumnInfo::ColumnType) + sizeof(size_t) + varcharSize;
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cerr << "[ERROR] Unreachable" << std::endl;
#endif
  VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  exit(VerdantStatus::INTERNAL_ERROR);
}

std::string ColumnInfo::toString() {
  std::string result;
  switch (type) {
    case ColumnInfo::INT:
      result += "INT";
      break;
    case ColumnInfo::FLOAT:
      result += "FLOAT";
      break;
    case ColumnInfo::VARCHAR:
      result += "VARCHAR(" + std::to_string(varcharSize) + ")";
      break;
  }
  if (isPrimary) {
    result += " PRIMARY KEY";
  }
  return result;
}
