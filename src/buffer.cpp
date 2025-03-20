#include "buffer.hpp"

Buffer::Buffer() : data(nullptr), size(0) {}
Buffer::Buffer(const char *data, std::size_t size) : data(data), size(size) {}

const char *const Buffer::getData() { return this->data; }
const std::size_t Buffer::getSize() { return this->size; }
