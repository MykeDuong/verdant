#pragma once

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "optional.h"
#include "parameters.h"
#include "status.h"
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
  assert(this->order > 0);
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] B-Tree order: " << this->order << std::endl;
#endif
  this->root = nullptr;
}

template <typename T> BTree<T>::~BTree() {
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] Deallocating BTree" << std::endl;
#endif 
  if (this->root.get() == nullptr) {
    return;
  }
}

template <typename T> bool BTree<T>::insert(T value) {
  if (this->root == nullptr) {
    this->root = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>(this->order));
  }
  std::pair<Optional<T>, std::unique_ptr<BTreeNode<T>>> result = this->root->insert(value);
  if (result.first.error != 0) { // Nothing to add
    return true;
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] BTree: Starting root replacement." << std::endl;
#endif
  T newValue = result.first.unwrap();
  std::unique_ptr<BTreeNode<T>> newNode = std::move(result.second);
  std::unique_ptr<BTreeNode<T>> newRoot(new BTreeNode<T>(order, nullptr));
  this->root->parent = newRoot.get();
  newNode->parent = newRoot.get();
  newRoot->children.push_back(std::move(this->root));
  newRoot->children.push_back(std::move(newNode));
  newRoot->values.push_back(newValue);
  this->root = std::move(newRoot);
  return true;
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
  return this->root->validate(true).first;
}

template <typename T> size_t BTree<T>::getHeight() {
#ifdef VERDANT_FLAG_DEBUG
  assert(this->validate());
#endif
  return this->root->getHeight();
}

template <typename T> size_t BTree<T>::countNodes() {
  return this->root->countNodes();
}

template <typename T> double BTree<T>::getAverageKeyPerNode() {
  size_t numberOfKeys = this->root->countKeys();
  size_t numberOfNodes = this->countNodes();

  return (double)numberOfKeys / numberOfNodes;
}

// B-Tree Node Anatomy
// ptr -- key -- ptr -- key -- ... -- key -- ptr -- next_block_ptr
// 2M + 1 ptr, 2M key, 1 next_block_ptr
// -> 2M + 2 ptr, 2M key

template <typename T> BTreeNode<T>::BTreeNode(size_t order, BTreeNode<T>* parent) {
  this->order = order;
  this->next = nullptr;
  this->parent = parent;
//  this->children.reserve(2 * this->order + 1);
//  this->values.reserve(2 * this->order);
}

template <typename T> size_t BTreeNode<T>::insertIndexSearch(T& value) {
  // Find the location to insert the value
  // Empty node
  if (this->values.size() == 0) {
    return 0;
  }

  size_t left = 0;
  size_t right = this->values.size() - 1;

  // Rightmost key is less than the value
  if (this->values[right] < value) {
    return right + 1;
  }

  // Leftmost key is larger than the value
  if (this->values[left] > value) {
    return 0;
  }

  // Find the one equal to the value, or the smallest one larger than the value
  while (left < right) {
    size_t mid = std::floor(right - left) / 2 + left;
    if (this->values[mid] == value) {
      return mid;
    }
    if (this->values[mid] < value) {
      left = mid + 1;
    } else { // this->values[mid] > value
      right = mid;
    }
  }
  return right;
}

template <typename T> bool BTreeNode<T>::isLeaf() {
  return this->children.size() == 0;
}

template <typename T> Optional<BTreeNode<T>*> BTreeNode<T>::getPrevChild(BTreeNode* curChild) {
#ifdef VERDANT_FLAG_DEBUG
  assert(curChild->parent == this);
#endif
  for (auto childItr = this->children.begin(); childItr != this->children.end(); childItr++) {
    if ((*childItr).get() == curChild) {
      return childItr != this->children.begin() ? Optional<BTreeNode<T>*>((childItr - 1)->get()) : Optional<BTreeNode<T>*>();
    }
  }
  std::cout << "[ERROR] Internal Error occurred" << std::endl;
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[ERROR] Sibling not found. Invalid B-Tree." << std::endl;
#endif
  VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  exit(VerdantStatus::INTERNAL_ERROR);
}

template <typename T> Optional<BTreeNode<T>*> BTreeNode<T>::getNextChild(BTreeNode* curChild) {
#ifdef VERDANT_FLAG_DEBUG
  assert(curChild->parent == this);
#endif
  for (auto childItr = this->children.begin(); childItr != this->children.end(); childItr++) {
    if ((*childItr).get() == curChild) {
      return childItr != this->children.end() - 1 ? Optional<BTreeNode<T>*>((childItr + 1)->get()) : Optional<BTreeNode<T>*>();
    }
  }
  std::cout << "[ERROR] Internal Error occurred" << std::endl;
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[ERROR] Sibling not found. Invalid B-Tree." << std::endl;
#endif
  VerdantStatus::handleError(VerdantStatus::INTERNAL_ERROR);
  exit(VerdantStatus::INTERNAL_ERROR);
}
// 8
// | 1 | 3 | 5 | 7 |
// Add 2 -> insertIndex = 1
// | 1 | 3 |

template <typename T> std::pair<Optional<T>, std::unique_ptr<BTreeNode<T>>> BTreeNode<T>::insert(T value) {
  size_t insertIndex = this->insertIndexSearch(value);
  std::unique_ptr<BTreeNode<T>> ptr = nullptr;
  if (!this->isLeaf()) {
    BTreeNode<T>* recursiveChild = this->children[(insertIndex < this->values.size() && this->values[insertIndex] == value) ? insertIndex + 1 : insertIndex].get();
    std::pair<Optional<T>, std::unique_ptr<BTreeNode<T>>> recursiveResult = recursiveChild->insert(value);
    if (recursiveResult.first.error != 0) { // Nothing to add/execute. Return
      return recursiveResult;
    }
    value = recursiveResult.first.unwrap();
    ptr = std::move(recursiveResult.second);
  }

  insertIndex = this->insertIndexSearch(value);
  assert(this->isLeaf() || insertIndex == this->values.size() || this->values[insertIndex] != value);

  if (this->values.size() < 2 * this->order || (insertIndex < this->values.size() && this->values[insertIndex] == value)) {
    if (insertIndex < this->values.size() && this->values[insertIndex] == value) {
      this->values[insertIndex] = value;
      assert(this->isLeaf() && ptr == nullptr);
    } else {
      assert(this->values.size() < 2 * this->order);
      this->values.insert(this->values.begin() + insertIndex, value);
      if (ptr != nullptr) {
        this->children.insert(this->children.begin() + insertIndex + 1, std::move(ptr));
      }
    }
    return std::make_pair(Optional<T>(), nullptr);
  }

  // TODO: Borrow space from prev and next siblings
  /**
  auto prevChild = this->parent->getPrevChild(this);
  if (prevChild.error == 0) {
    
  }

  auto nextChild = this->parent->getNextChild(this);
  if (nextChild.error == 0) {
    
  }
  **/

  // Node is full. Splitting. Assumming distinct elements
  std::unique_ptr<BTreeNode<T>> newNode(new BTreeNode<T>(this->order, this->parent));
  auto oldNext = this->next;
  this->next = newNode.get();
  newNode->next = oldNext;

  // Add value
  T median;
  if (insertIndex == this->order) {
    median = value;
  } else if (insertIndex < this->order) {
    median = this->values[this->order - 1];
  } else {
    median = this->values[this->order];
  }
  bool valueAdded = false;
  size_t valuesMoved = 0;
  size_t childrenMoved = 0;
  for (size_t i = this->order - 1; i < 2 * this->order; i++) {
    if (this->values[i] < median) {
      continue;
    }
    if (!this->isLeaf() && this->values[i] == median) {
      newNode->children.push_back(std::move(this->children[i]));
      childrenMoved++;
      continue;
    }
    if (!valueAdded && ((value > median) || (value == median && this->isLeaf())) && this->values[i] > value) {
      newNode->values.push_back(value);
      if (!this->isLeaf()) {
        newNode->children.push_back(std::move(ptr));
      }
      valueAdded = true;
    } 
    if (!this->isLeaf()) {
      newNode->children.push_back(std::move(this->children[i]));
      childrenMoved++;
    }
    newNode.get()->values.push_back(this->values[i]);
    valuesMoved++;
  }
  if (!valueAdded && value < median) {
    this->values.insert(this->values.begin() + insertIndex, value);
  }
  this->values.resize(this->values.size() - valuesMoved);
  if (!this->isLeaf()) {
    this->children.resize(this->children.size() - childrenMoved);
  }
  return std::make_pair(Optional<T>(median), std::move(newNode));
}

// Order 2
// 1, 3, 5, 7
// Add 0 -> median = 3
// Add 2 -> index 1 -> median = [2 - 1] = [1] -> 3
// Add 4 -> index = 2 -> order = index -> median = 4
// Add 6 -> index = 3 -> median = [2] = 5
// Add 8 -> median = 5

template <typename T> bool BTreeNode<T>::remove(T& value) {
  return true;
}


template <typename T> Optional<T> BTreeNode<T>::search(T& value) {
  if (this->values.size() == 0) {
    return Optional<T>();
  }

  size_t insertIndex = this->insertIndexSearch(value);

  if (insertIndex != this->values.size() || this->values[insertIndex] == value) {
    return this->values[insertIndex];
  } else {
    // Assert that the search works as expected
    assert(insertIndex == this->values.size() || this->values[insertIndex] > value);
    assert(insertIndex == 0 || *this->values[insertIndex - 1] < value);

    return this->children[insertIndex]->search(value);
  }
}

template <typename T> std::pair<bool, size_t> BTreeNode<T>::validate(bool root, T* minVal, T* maxVal) {
  // NOTE: Rule 1: Each node contains at most 2M key
  if (this->values.size() > 2 * this->order) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] BTree failed by having more than 2 * order keys." << std::endl;
#endif
    return { false, SIZE_MAX };
  }

  // NOTE: Rule 2: Each node, except the root, has at least M keys
  if (!root && this->values.size() < this->order) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] BTree failed by having less than order keys in a non-root node." << std::endl;
#endif
    return { false, SIZE_MAX };
  }

  if (this->values.size() > 0) {
    // Check that the first key is larger or equal to minVal, and the last key is less than maxVal
    if (minVal != nullptr && this->values[0] < *minVal) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[ERROR] BTree failed by having first key of a node less than previous key of its parent node." << std::endl;
#endif
      return { false, SIZE_MAX };
    }

    if (maxVal != nullptr && this->values[this->values.size() - 1] >= *maxVal) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[ERROR] BTree failed by having last key of a node larger than or equal to previous key of its parent node." << std::endl;
#endif
      return { false, SIZE_MAX };
    }
    
    // Check that the keys are in ascending order
    for (size_t i = 0; i < this->values.size() - 1; i++) {
      if (this->values[i] > this->values[i + 1]) {
#ifdef VERDANT_FLAG_DEBUG
        std::cout << "[DEBUG] Number of keys: " << this->values.size() << std::endl;
        std::cout << "[ERROR] BTree failed by having non-ascending order node at key " << i << std::endl;
#endif
        return { false, SIZE_MAX };
      }
    }
  }


  size_t childDepth = 0;
  size_t numChild = 0;

  for (size_t i = 0; i < this->children.size(); i++) {
    if (this->children[i] != nullptr) {
      numChild += 1;
      std::pair<bool, size_t> childResult = this->children[i]->validate(false, i == 0 ? nullptr : &this->values[i - 1], i == this->children.size() - 1 ? nullptr : &this->values[i]);
      if (childResult.first == false) {
        return { false, SIZE_MAX };
      }
      if (childDepth == 0) {
        childDepth = childResult.second;
      // NOTE: Rule 4: All leaf nodes are at the same depth (balanced)
      } else if (childDepth != childResult.second) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] BTree is not balanced" << std::endl;
#endif
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
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] Non-leaf node has less than 2 children" << std::endl;
#endif
      return { false, SIZE_MAX };
    }
    // NOTE: Rule 5: A non-leaf node with n keys has exactly n + 1 children
    if (numChild != this->values.size() + 1) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] Mismatched keys-children values" << std::endl;
#endif
      return { false, SIZE_MAX };
    }
    return { true, 0 };
  }
}

template <typename T> size_t BTreeNode<T>::getHeight() {
  if (this->isLeaf()) {
    return 1;
  }
  return this->children[0]->getHeight() + 1;
}

template <typename T> size_t BTreeNode<T>::countNodes() {
  size_t total = 1; // Including itself
  
  for (auto child = this->children.begin(); child != this->children.end(); child++) {
    total += (*child)->countNodes();
  }

  return total;
}

template <typename T> size_t BTreeNode<T>::countKeys() {
  size_t total = this->values.size(); // Including itself
  
  for (auto child = this->children.begin(); child != this->children.end(); child++) {
    total += (*child)->countKeys();
  }

  return total;
}
