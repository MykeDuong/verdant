#pragma once

#include "absl/status/statusor.h"
#include "btree_node.hpp"
#include "file_operator.hpp"

#include <unordered_map>
#include <vector>

template <typename T> class BTree;

template <typename T> class BTree {
private:
  friend class BTreeNode<T>;
  FileOperator &fileOp;
  std::size_t order;
  std::unique_ptr<BTreeNode<T>> root;
  
  std::unordered_map<BTreeNode<T>*, std::size_t> nodePositions;

  bool isEmpty();
#ifdef VERDANT_FLAG_BTREE_TEST
  std::size_t nextId;
#endif

public:
  BTree(FileOperator& fileOp, std::size_t root);
  ~BTree();

  absl::StatusOr<std::size_t> insert(T value);

  absl::StatusOr<T> remove(const T &value);

  absl::StatusOr<T> search(const T &value);

  absl::StatusOr<std::vector<T>> searchRange(const T &minVal, const T &maxVal);

  bool validate();

  absl::StatusOr<T> getMinValue();

  absl::StatusOr<T> getMaxValue();

  size_t getHeight();

  size_t countNodes();

  double getAverageKeyPerNode();
};

#include "btree.cpp"
