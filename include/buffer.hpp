#pragma once

#include <cstddef>

class Buffer {
private:
  const char *data;
  std::size_t size;

public:
  Buffer();
  Buffer(const char *data, std::size_t size);
  const char *const getData();
  const std::size_t getSize();
};
