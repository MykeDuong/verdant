#pragma once

#include "absl/status/statusor.h"
#include "parameters.hpp"

#include <cstddef>
#include <memory>
#include <vector>

template <typename T> class BTree;

template <typename T> class BTreeNode {
public:
  BTreeNode(BTree<T>& tree, BTreeNode *parent = nullptr);
  BTreeNode(BTree<T>& tree, char* data, BTreeNode *parent = nullptr);

private:
  friend BTree<T>;
  BTree<T>& tree;
  BTreeNode<T> *parent;
  BTreeNode *next;
  std::vector<std::unique_ptr<BTreeNode<T>>> children;
  std::vector<T> values;
  char data[Parameter::BLOCK_SIZE];
#ifdef VERDANT_FLAG_BTREE_TEST
  std::size_t id;
  std::size_t nextId;
#endif
  std::size_t getOrder();

  size_t insertIndexSearch(const T &value);

  bool isLeaf();

  bool isRoot();

  bool isFull();

  bool isLeast();

  size_t findChildIndex(BTreeNode<T> *child);

  absl::StatusOr<BTreeNode<T> *> getPrevChild(BTreeNode *curChild);

  absl::StatusOr<BTreeNode<T> *> getNextChild(BTreeNode *curChild);

  std::pair<absl::StatusOr<T>, std::unique_ptr<BTreeNode<T>>> insert(T value);

  std::pair<absl::StatusOr<T>, absl::StatusOr<size_t>> remove(const T &value);

  std::pair<absl::StatusOr<T>, absl::StatusOr<size_t>> removeOnLeaf(const T &value);

  std::pair<absl::StatusOr<T>, absl::StatusOr<size_t>> removeOnInternal(const T &value);

  absl::StatusOr<T> search(const T &value);

  absl::StatusOr<std::vector<T>> searchRange(const T &minVal, const T &maxVal);

  std::pair<bool, size_t> validate(bool root = false,
                                   BTreeNode *parent = nullptr,
                                   T *minVal = nullptr, T *maxVal = nullptr);

  absl::StatusOr<T> getMinValue();

  absl::StatusOr<T> getMaxValue();

  size_t getHeight();

  size_t countNodes();

  size_t countKeys();

  static void mergeLeafNodes(BTreeNode<T> *first, BTreeNode<T> *second);

  static void mergeInternalNodes(BTreeNode<T> *first, const T middleValue,
                                 BTreeNode<T> *second);
};

#include "btree_node.cpp"
