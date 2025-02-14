#pragma once

#include "status.hpp"
template <typename T> class Optional {
private:
  T value;

public:
  VerdantStatus::StatusEnum status;

  Optional();
  Optional(VerdantStatus::StatusEnum error);
  Optional(const T value);
  Optional(const Optional &op) = delete;
  Optional(Optional &&optional);

  bool unwrappable();
  void setValue(const T &&value);
  T unwrap();
  T &peek();
};

#include "optional.cpp"
