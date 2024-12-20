#pragma once

#include "optional.h"
#include <memory>

template <typename T>
class BTreeNode{
private:
  size_t order;
  size_t used;
  char* array;

  Optional<BTreeNode<T>*> getPointerAtIndex(size_t index);
  Optional<T*> getKeyAtIndex(size_t index);
  size_t oneNodeIndexSearch(T& value);
public:
  BTreeNode(size_t order);
  ~BTreeNode();

  bool insert(T& value);

  bool remove(T& value);

  Optional<T> search(T& value);

  std::pair<bool, size_t> validate(bool root = false);
};

template <typename T>
class BTree {
private:
  size_t order;
  std::unique_ptr<BTreeNode<T>> root;
public:
  BTree();

  bool insert(T& value);

  bool remove(T& value);

  Optional<T> search(T& value);

  bool validate();
};

#include "btree.cpp"
