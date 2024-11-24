#pragma once

template <typename T> class Optional {
private:
  T value;

public:
  int error;

  Optional();
  Optional(T value);
  Optional(Optional &&optional);
  T unwrap();
};

#include "optional.cpp"
