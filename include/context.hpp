#pragma once

#include "absl/status/statusor.h"

#include <string>

struct Context {
  absl::StatusOr<std::string> database;
  absl::StatusOr<std::string*> statement;
};
