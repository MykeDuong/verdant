#include "status.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

const std::string VerdantStatus::errorCodeToString(VerdantStatus::StatusEnum status) {
  switch (status) {
  case VerdantStatus::INTERNAL_ERROR:
    return "INTERNAL_ERROR";
  case VerdantStatus::TERMINATED:
    return "TERMINATED";
  case VerdantStatus::SUCCESS:
    return "SUCCESS";
  case VerdantStatus::INVALID_PERMISSION:
    return "INVALID_PERMISSION";
  case VerdantStatus::INVALID_SYNTAX:
    return "INVALID_SYNTAX";
  case VerdantStatus::INVALID_TYPE:
    return "INVALID_TYPE";
  case VerdantStatus::OUT_OF_BOUND:
    return "OUT_OF_BOUND";
  case VerdantStatus::UNSPECIFIED_DATABASE:
    return "UNSPECIFIED_DATABASE";
  case VerdantStatus::GENERIC_ERROR:
    return "GENERIC_ERROR";
  case VerdantStatus::UNIMPLEMENTED:
    return "UNIMPLEMENTED";
  }
  return "UNKNOWN_ERROR";
}

void VerdantStatus::handleError(VerdantStatus::StatusEnum status) {
  std::cerr << "[ERROR] An unrecoverable error has occurred with error code "
            << VerdantStatus::errorCodeToString(status) << "." << std::endl;
  std::exit(status);
}
