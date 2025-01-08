#pragma once

#include "status.h"
template <typename T>
class Optional {
  T value;

public:
  VerdantStatus::StatusEnum status;

  Optional();
  Optional(VerdantStatus::StatusEnum error);
  Optional(T value);
  Optional(const Optional& op) = delete;
  Optional(Optional&& optional);

  bool unwrappable();
  void setValue(T&& value);
  T unwrap();
  T& peek();
};

#include "optional.cpp"
