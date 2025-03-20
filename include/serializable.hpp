#pragma once

#include "buffer.hpp"

class Serializable {
  virtual Buffer serialize() = 0;
  virtual ~Serializable() = 0;
};
