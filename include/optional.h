#pragma once

template <typename T>
class Optional {
  T value;

public:
  int error;

  Optional();
  Optional(T value);
  Optional(Optional& op) = delete;
  Optional(Optional&& optional);
  void setValue(T&& value);
  T unwrap();

};

#include "optional.cpp"
