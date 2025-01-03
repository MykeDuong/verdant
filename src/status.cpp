#include "status.h"

#include <cstdlib>
#include <iostream>
#include <string>

// TODO: Better error handling
void VerdantStatus::handleError(VerdantStatus::StatusEnum status) {
  std::string statusString;

  switch (status) {
  case VerdantStatus::INTERNAL_ERROR: {
    statusString = "INTERNAL_ERROR";
    break;
  }
  case VerdantStatus::SUCCESS: {
    statusString = "SUCCESS";
    break;
  }
  case VerdantStatus::INVALID_PERMISSION: {
    statusString = "INVALID PERMISSION";
    break;
  }
  }

  std::cerr << "[ERROR] An unrecoverable error has occurred with error code "
            << statusString << "." << std::endl;
  std::exit(status);
}
