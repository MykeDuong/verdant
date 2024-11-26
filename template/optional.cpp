#pragma once

#include "optional.h"
#include <status.h>
#include <iostream>
#include <utility>

template <typename T>
Optional<T>::Optional() {
  this->error = -1;
}

template <typename T>
Optional<T>::Optional(T value) {
  this->value = std::move(value);
  this->error = 0;
}

template <typename T>
Optional<T>::Optional(Optional&& optional) {
  this->value = std::move(optional.value);
  this->error = std::move(optional.error);
}

template <typename T>
void Optional<T>::setValue(T&& value) {
  this->value = std::move(value);
}

template <typename T>
T Optional<T>::unwrap() {
  if (this->error != 0) {
#ifdef VERDANT_FLAG_DEBUG
    std::cerr << "[DEBUG] Error trying to unwrap a null optional value with error code " 
              << this->error << "." << std::endl;
#endif
    VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  }
  return std::move(this->value);
}
