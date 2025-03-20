#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "btree.hpp"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "btree_node.hpp"
#include "file_operator.hpp"
#include "parameters.hpp"


template <typename T> BTree<T>::BTree(FileOperator& fileOp, std::size_t root) : fileOp(fileOp) {
#ifdef VERDANT_FLAG_BTREE_TEST
  std::cout << "[DEBUG] Size of pointer: " << sizeof(size_t) << std::endl;
  std::cout << "[DEBUG] Size of block: " << Parameter::BLOCK_SIZE << std::endl;
#endif
  assert(Parameter::BLOCK_SIZE > 2 * sizeof(size_t));
  // M = (BLOCK_SIZE - sizeof(char) - 3 * sizeof(size_t)) / (4 * sizeof(size_t) + 2 * sizeof(key))
  this->order = std::floor(
    (Parameter::BLOCK_SIZE - sizeof(char) - 3 * sizeof(std::size_t)) /
    (4 * sizeof(size_t) + 2 * sizeof(T))
  );
  assert(this->order > 0);
#ifdef VERDANT_FLAG_BTREE_TEST
  std::cout << "[DEBUG] B-Tree order: " << this->order << std::endl;
#endif
  this->root = std::make_unique<BTreeNode<T>>(*this);
}

template <typename T> BTree<T>::~BTree() {
#ifdef VERDANT_FLAG_BTREE_TEST
  std::cout << "[DEBUG] Deallocating BTree" << std::endl;
#endif
  if (this->isEmpty()) {
    return;
  }
}

template <typename T> bool BTree<T>::isEmpty() {
  return this->root == nullptr;
}

template <typename T> absl::StatusOr<std::size_t> BTree<T>::insert(T value) {
  if (this->isEmpty()) {
    this->root = std::make_unique<BTreeNode<T>>(*this);
  }
  std::pair<absl::StatusOr<T>, std::unique_ptr<BTreeNode<T>>> result = this->root->insert(value);
  if (!result.first.ok()) { // Nothing to add
    return true;
  }
#ifdef VERDANT_FLAG_BTREE_TEST
  std::cout << "[DEBUG] BTree: Starting root replacement." << std::endl;
#endif
  T newValue = result.first.value();
  std::unique_ptr<BTreeNode<T>> newNode = std::move(result.second);
  std::unique_ptr<BTreeNode<T>> newRoot(new BTreeNode<T>(*this));
  this->root->parent = newRoot.get();
  newNode->parent = newRoot.get();
  newRoot->children.push_back(std::move(this->root));
  newRoot->children.push_back(std::move(newNode));
  newRoot->values.push_back(newValue);
  this->root = std::move(newRoot);
#ifdef VERDANT_FLAG_BTREE_TEST
  assert(this->root->children[0]
             ->values[this->root->children[0]->values.size() - 1] < newValue);
  assert(newValue <= this->root->children[1]->values[0]);
  std::cout << "[DEBUG] Current root ID: " << this->root->index << std::endl;
  std::cout << "[DEBUG] Old root ID: " << this->root->children[0]->index
            << std::endl;
  std::cout << "[DEBUG] New node ID: " << this->root->children[1]->index
            << std::endl;
  std::cout << "[DEBUG] BTree: Finished root replacement." << std::endl;
#endif
  return true;
}

template <typename T> absl::StatusOr<T> BTree<T>::remove(const T &value) {
  if (this->isEmpty()) {
    return absl::NotFoundError("No node found");
  }
  auto nodeOperationResult = this->root->remove(value);
  auto result = std::move(nodeOperationResult.first);
  auto optionalRemovedIndex = std::move(nodeOperationResult.second);

  assert(!optionalRemovedIndex.ok());

  if (this->root->values.size() == 0) {
#ifdef VERDANT_FLAG_BTREE_TEST
    std::cout << "[DEBUG] Performing root replacement process" << std::endl;
#endif
    if (this->root->isLeaf()) {
      this->root = nullptr;
    } else {
      this->root = std::move(this->root->children[0]);
      this->root->parent = nullptr;
    }
  }

  return result;
}

template <typename T> absl::StatusOr<T> BTree<T>::search(const T &value) {
  auto result = this->root->search(value);
  return result;
}

template <typename T>
absl::StatusOr<std::vector<T>> BTree<T>::searchRange(const T &minVal,
                                               const T &maxVal) {
  if (this->root== nullptr) {
    return absl::NotFoundError("B-Tree is empty");
  }

  return this->root->searchRange(minVal, maxVal);
}

template <typename T> bool BTree<T>::validate() {
#ifdef VERDANT_FLAG_BTREE_TEST
  std::cout << "[DEBUG] BTree Validation process started" << std::endl;
#endif
  if (this->root== nullptr) {
    return true;
  }
  return this->root->validate(true).first;
}

template <typename T> absl::StatusOr<T> BTree<T>::getMinValue() {
  if (this->root== nullptr) {
    return absl::NotFoundError("B-Tree is empty");
  }
  return this->root->getMinValue();
}

template <typename T> absl::StatusOr<T> BTree<T>::getMaxValue() {
  if (this->root== nullptr) {
    return absl::NotFoundError("B-Tree is empty");
  }
  return this->root->getMaxValue();
}

template <typename T> size_t BTree<T>::getHeight() {
  if (this->root== nullptr) {
    return 0;
  }
  return this->root->getHeight();
}

template <typename T> size_t BTree<T>::countNodes() {
  if (this->root== nullptr) {
    return 0;
  }
  return this->root->countNodes();
}

template <typename T> double BTree<T>::getAverageKeyPerNode() {
  if (this->root== nullptr) {
    return 0;
  }
  size_t numberOfKeys = this->root->countKeys();
  size_t numberOfNodes = this->countNodes();

  return (double)numberOfKeys / numberOfNodes;
}
