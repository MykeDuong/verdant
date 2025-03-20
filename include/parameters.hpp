#pragma once

#include <cstddef>

struct Parameter {
  static constexpr std::size_t BLOCK_SIZE = 8192;
  static constexpr char DATA_PATH[] = "~/.verdant/";
};
