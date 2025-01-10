#pragma once

#include "optional.h"
#include <status.h>
#include <iostream>
#include <utility>

template <typename T> Optional<T>::Optional() {
  this->status = VerdantStatus::GENERIC_ERROR;
}

template <typename T> Optional<T>::Optional(VerdantStatus::StatusEnum status) {
  this->status = status;
}

template <typename T>
Optional<T>::Optional(T value) {
  this->value = std::move(value);
  this->status = VerdantStatus::SUCCESS;
}

template <typename T>
Optional<T>::Optional(Optional&& optional) {
  this->value = std::move(optional.value);
  this->status = std::move(optional.status);
}

template <typename T> bool Optional<T>::unwrappable() {
  return this->status == VerdantStatus::SUCCESS;
}

template <typename T> void Optional<T>::setValue(const T&& value) {
  this->value = std::move(value);
  this->status = VerdantStatus::SUCCESS;
}

template <typename T> T Optional<T>::unwrap() {
  if (!this->unwrappable()) {
#ifdef VERDANT_FLAG_DEBUG
    std::cerr << "[DEBUG] Error trying to unwrap a null optional value with status code " 
              << this->status << "." << std::endl;
#endif
    VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  }
  return std::move(this->value);
}

template <typename T> T& Optional<T>::peek() {
  if (!this->unwrappable()) {
#ifdef VERDANT_FLAG_DEBUG
    std::cerr << "[DEBUG] Error trying to unwrap a null optional value with status code " 
              << this->status << "." << std::endl;
#endif
    VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  }
  return this->value;
}
