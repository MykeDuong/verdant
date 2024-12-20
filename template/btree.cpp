#pragma once

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "optional.h"
#include "parameters.h"
#include "btree.h"

// BTreeNode size
// Order = M -> 2M (key) + (2M + 1) (ptr_size) + next_block_ptr_size + used_size = block_size
// M = (block_size - 2 * ptr_size - used_size) / (2 * (key + ptr_size))
// M = (block_size - 3 * sizeof(size_t)) / (2 * (key + ptr_size))

template <typename T> BTree<T>::BTree() {
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] Size of pointer: " << sizeof(size_t) << std::endl;
  std::cout << "[DEBUG] Size of block: " << BLOCK_SIZE << std::endl;
#endif
  assert(BLOCK_SIZE > 2 * sizeof(size_t));
  this->order = std::floor((BLOCK_SIZE - 3 * sizeof(size_t)) / (2 * (sizeof(T) + sizeof(size_t))));
  assert(order > 0);
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] B-Tree order: " << this->order << std::endl;
#endif
  this->root = nullptr;
}

template <typename T> bool BTree<T>::insert(T& value) {
  if (this->root == nullptr) {
    this->root = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>(this->order));
  }
  bool result = this->root->insert(value);
  return result;
}

template <typename T> bool BTree<T>::remove(T& value) {
  bool result = this->root->remove(value);
  return result;
}

template <typename T> Optional<T> BTree<T>::search(T& value) {
  bool result = this->root->remove(value);
  return result;
}

template <typename T> bool BTree<T>::validate() {
  if (this->root == nullptr) {
    return true;
  }
  return this->root->validate(true)[0];
}

// B-Tree Node Anatomy
// ptr -- key -- ptr -- key -- ... -- key -- ptr -- next_block_ptr
// 2M + 1 ptr, 2M key, 1 next_block_ptr
// -> 2M + 2 ptr, 2M key

template <typename T> BTreeNode<T>::BTreeNode(size_t order) {
  this->order = order;
  this->array = (char*)calloc((2 * order + 2) * sizeof(size_t) + 2 * order * sizeof(T), sizeof(char));
  this->used = 0;
}

template <typename T> BTreeNode<T>::~BTreeNode() {
  for (size_t i = 0; i < this->used + 1; i++) {
    BTreeNode* child = this->getPointerAtIndex(i).unwrap();
    delete child;
  }
  free(this->array);
}

template <typename T> Optional<T*> BTreeNode<T>::getKeyAtIndex(size_t index) {
  size_t numberOfKeys = 2 * this->order;
  if (index >= numberOfKeys) {
    Optional<T*> error(nullptr);
    error.error = 1;
    return error;
  }
  // There is (index + 1) ptr and (index) keys before the key
  char* pos = this->array + (index + 1) * sizeof(size_t) + (index) * sizeof(T);
  Optional<T*> result((T*)pos);
  return result;
}

template <typename T> Optional<BTreeNode<T>*> BTreeNode<T>::getPointerAtIndex(size_t index) {
  size_t numChildren = this->used + 1;
  if (index >= numChildren) {
    Optional<BTreeNode*> error;
    error.error = 1;
    return error;
  }
  // There is (index) ptr and (index) keys before the ptr
  char* pos = this->array + index * sizeof(size_t) + index * sizeof(T);
  Optional<BTreeNode<T>*> result((BTreeNode<T>*)pos);
  return result;
}

template <typename T> size_t BTreeNode<T>::oneNodeIndexSearch(T& value) {
  // Find the location to insert the value
  // Empty node
  if (this->used == 0) {
    return 0;
  }

  size_t left = 0;
  size_t right = this->used - 1;

  // Rightmost key is less than the value
  if (*(this->getKeyAtIndex(right).unwrap()) < value) {
    return right + 1;
  }

  // Leftmost key is larger than the value
  if (*(this->getKeyAtIndex(left).unwrap()) > value) {
    return 0;
  }

  // Find the one equal to the value, or the smallest one larger than the value
  while (left < right) {
    size_t mid = std::floor(right - left) / 2 + left;
    if (*this->getKeyAtIndex(mid).unwrap() == value) {
      return mid;
    }
    if (*this->getKeyAtIndex(mid).unwrap() < value) {
      left = mid + 1;
    } else { // getKeyAtIndex(mid) > value
      right = mid;
    }
  }
  return right;
  
}

template <typename T> bool BTreeNode<T>::insert(T& value) {
  this->used++;
  return true;
}

template <typename T> bool BTreeNode<T>::remove(T& value) {
  this->used--;
  return true;
}


template <typename T> Optional<T> BTreeNode<T>::search(T& value) {
  if (this->used == 0) {
    Optional<T> result;
    result.error = 1;
    return result;
  }

  size_t insertIndex = this->oneNodeIndexSearch(value);

  if (insertIndex != this->used || *this->getKeyAtIndex(insertIndex).unwrap() == value) {
    return Optional<T>(*this->getKeyAtIndex(insertIndex).unwrap());
  } else {
    // Assert that the search works as expected
    assert(insertIndex == this->used || *this->getKeyAtIndex(insertIndex).unwrap() > value);
    assert(insertIndex == 0 || *this->getKeyAtIndex(insertIndex - 1).unwrap() < value);


    return (this->getPointerAtIndex(insertIndex).unwrap())->search(value);
  }
}

template <typename T> std::pair<bool, size_t> BTreeNode<T>::validate(bool root) {
  // NOTE: Rule 1: Each node contains at most 2M key
  if (this->used > 2 * this->order) {
    return { false, SIZE_MAX };
  }

  // NOTE: Rule 2: Each node, except the root, has at least M keys
  if (!root && this->used < this->order) {
    return { false, SIZE_MAX };
  }

  // Check that the keys are in ascending order
  for (size_t i = 0; i < this->used - 1; i++) {
    if (*this->getKeyAtIndex(i).unwrap() > *this->getKeyAtIndex(i + 1).unwrap()) {
      return { false, SIZE_MAX };
    }
  }
  size_t childDepth = -1;
  size_t numChild = 0;

  for (size_t i = 0; i < this->used + 1; i++) {
    if (this->getPointerAtIndex(i).unwrap() != nullptr) {
      numChild += 1;
      std::pair<bool, size_t> childResult = (BTreeNode*)(*this->getPointerAtIndex(i))->validate();
      if (childResult.first == false) {
        return { false, SIZE_MAX };
      }
      if (childDepth == -1) {
        childDepth = childResult.second;
      // NOTE: Rule 4: All leaf nodes are at the same depth (balanced)
      } else if (childDepth != childResult.second) {
        return { false, SIZE_MAX };
      }
    }
  }
  
  if (numChild == 0) {
    // Leaf node
    return { true, 0 };
  } else {
    // Internal node
    // NOTE: Rule 3: All non-leaf node have at least 2 children
    if (numChild < 2) {
      return { false, SIZE_MAX };
    }
    // NOTE: Rule 5: A non-leaf node with n keys has exactly n + 1 children
    if (numChild != this->used + 1) {
      return { false, SIZE_MAX };
    }
    return { true, 0 };
  }
}
