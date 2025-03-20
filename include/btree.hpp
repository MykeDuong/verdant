#pragma once

#include "absl/status/statusor.h"
#include "btree_node.hpp"
#include "file_operator.hpp"

#include <unordered_map>
#include <vector>

// BTreeNode size
// Order = M -> 2M (key) + (2M + 1) (ptr_size) + next_block_ptr_size + used_size = block_size
// M = (block_size - 2 * ptr_size - used_size) / (2 * (key +
// ptr_size)) 
// M = (block_size - 3 * sizeof(size_t)) / (2 * (key + ptr_size))


// BTree Node Anatomy:
// [status][key_count][rightmost_ptr][next_ptr][ptr_pairs]...[pairs]
// Order = M
// 1 char of status (currently in used, freed, etc)
// 1 size_t number of keys currently in tree
// 1 rightmost ptr
// 1 next block ptr
// pointers to pairs
// [ptr - key] pairs
// sizeof(status) + sizeof(keyCount) + 2M * sizeof(ptr) + 2M * sizeof(key) + 2M * sizeof(ptr) + 2 * sizeof(ptr)
// sizeof(char) + sizeof(size_t) + 2M*sizeof(size_t) + 2M * sizeof(key) + 2M * sizeof(size_t) + 2 * sizeof(size_t)
// sizeof(char) + (4M + 3) * sizeof(size_t) + 2M * sizeof(key) = BLOCK_SIZE
// M * (4 * sizeof(size_t) + 2 * sizeof(key)) = BLOCK_SIZE - sizeof(char) - 3 * sizeof(size_t)
// M = (BLOCK_SIZE - sizeof(char) - 3 * sizeof(size_t)) / (4 * sizeof(size_t) + 2 * sizeof(key))
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
