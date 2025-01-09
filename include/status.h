#pragma once

#include <string>

class VerdantStatus {
public:
  typedef enum {
    SUCCESS = 0,
    TERMINATED,
    INTERNAL_ERROR,
    INVALID_PERMISSION,
    INVALID_SYNTAX,
    INVALID_TYPE,
    OUT_OF_BOUND,
    UNSPECIFIED_DATABASE,
    UNIMPLEMENTED,
    GENERIC_ERROR,
  } StatusEnum;

  static void handleError(VerdantStatus::StatusEnum status);
  static const std::string errorCodeToString(VerdantStatus::StatusEnum status);
private:
  static VerdantStatus::StatusEnum value;
};

