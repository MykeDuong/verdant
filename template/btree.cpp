#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#ifdef VERDANT_FLAG_DEBUG 
#include <unordered_set>
#endif

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
  if (!result.first.unwrappable()) { // Nothing to add
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
#ifdef VERDANT_FLAG_DEBUG
  assert(this->root->children[0]->values[this->root->children[0]->values.size() - 1] < newValue);
  assert(newValue <= this->root->children[1]->values[0]);
  std::cout << "[DEBUG] Current root ID: " << this->root->index << std::endl;
  std::cout << "[DEBUG] Old root ID: " << this->root->children[0]->index << std::endl;
  std::cout << "[DEBUG] New node ID: " << this->root->children[1]->index << std::endl;
  std::cout << "[DEBUG] BTree: Finished root replacement." << std::endl;
#endif
  return true;
}

template <typename T> Optional<T> BTree<T>::remove(const T& value) {
  if (this->root == nullptr) {
    return Optional<T>();
  }
  auto nodeOperationResult = this->root->remove(value);
  auto result = std::move(nodeOperationResult.first);
  auto optionalRemovedIndex = std::move(nodeOperationResult.second);

  assert(!optionalRemovedIndex.unwrappable());

  if (this->root->values.size() == 0) {
#ifdef VERDANT_FLAG_DEBUG
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

template <typename T> Optional<T> BTree<T>::search(const T& value) {
  auto result = this->root->search(value);
  return result;
}

template <typename T>  Optional<std::vector<T>> BTree<T>::searchRange(const T& minVal, const T& maxVal) {
  if (this->root == nullptr) {
    return Optional<std::vector<T>>();
  }

  return this->root->searchRange(minVal, maxVal);
}

template <typename T> bool BTree<T>::validate() {
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] BTree Validation process started" << std::endl;
#endif
  if (this->root == nullptr) {
    return true;
  }
  return this->root->validate(true).first;
}

template <typename T> Optional<T> BTree<T>::getMinValue() {
  if (this->root == nullptr) {
    return Optional<T>();
  }
  return this->root->getMinValue();
}

template <typename T> Optional<T> BTree<T>::getMaxValue() {
  if (this->root == nullptr) {
    return Optional<T>();
  }
  return this->root->getMaxValue();
}

template <typename T> size_t BTree<T>::getHeight() {
  if (this->root == nullptr) {
    return 0;
  }
  return this->root->getHeight();
}

template <typename T> size_t BTree<T>::countNodes() {
  if (this->root == nullptr) {
    return 0;
  }
  return this->root->countNodes();
}

template <typename T> double BTree<T>::getAverageKeyPerNode() {
  if (this->root == nullptr) {
    return 0;
  }
  size_t numberOfKeys = this->root->countKeys();
  size_t numberOfNodes = this->countNodes();

  return (double)numberOfKeys / numberOfNodes;
}


// NOTE: BTreeNode section

#ifdef VERDANT_FLAG_DEBUG
template <typename T> size_t BTreeNode<T>::nextIndex = 0;
#endif

// B-Tree Node Anatomy
// ptr -- key -- ptr -- key -- ... -- key -- ptr -- next_block_ptr
// 2M + 1 ptr, 2M key, 1 next_block_ptr
// -> 2M + 2 ptr, 2M key

template <typename T> BTreeNode<T>::BTreeNode(size_t order, BTreeNode<T>* parent) {
  this->order = order;
  this->next = nullptr;
  this->parent = parent;
#ifdef VERDANT_FLAG_DEBUG
  this->index = BTreeNode<T>::nextIndex;
  BTreeNode<T>::nextIndex++;
#endif
  this->children.reserve(2 * this->order + 1);
  this->values.reserve(2 * this->order);
}

template <typename T> size_t BTreeNode<T>::insertIndexSearch(const T& value) {
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
#ifdef VERDANT_FLAG_DEBUG
  assert(this->values[right - 1] < value && value <= this->values[right]);
#endif
  return right;
}

template <typename T> bool BTreeNode<T>::isLeaf() {
  return this->children.size() == 0;
}

template <typename T> bool BTreeNode<T>::isRoot() {
  return this->parent == nullptr;
}

template <typename T> bool BTreeNode<T>::isFull() {
  assert(this->values.size() <= 2 * this->order);
  return this->values.size() == 2 * this->order;
}

template <typename T> bool BTreeNode<T>::isLeast() {
  if (this->isRoot()) {
    return false;
  }
#ifdef VERDANT_FLAG_DEBUG
  assert(this->values.size() >= this->order);
#endif
  return this->values.size() == this->order;
}

template <typename T> size_t BTreeNode<T>::findChildIndex(BTreeNode<T>* child) {
  for (size_t i = 0; i < this->children.size(); i++) {
    if (this->children[i].get() == child) {
      return i;
    }
  }
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[ERROR] Child index not found in a BTree Node" << std::endl;
  exit(VerdantStatus::INTERNAL_ERROR);
#endif

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
    if (!recursiveResult.first.unwrappable()) { // Nothing to add/execute. Return
      return recursiveResult;
    }
    value = recursiveResult.first.unwrap();
    ptr = std::move(recursiveResult.second);
  }

  insertIndex = this->insertIndexSearch(value);
  assert(this->isLeaf() || insertIndex == this->values.size() || this->values[insertIndex] != value);

  if (!this->isFull() || (insertIndex < this->values.size() && this->values[insertIndex] == value)) {
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
    return { Optional<T>(), nullptr };
  }

  if (!this->isRoot()) {
    if (!this->isLeaf()) {
      // Borrow space from previous sibling
      auto optionalPrevSibling = this->parent->getPrevChild(this);
      if (optionalPrevSibling.unwrappable() && !optionalPrevSibling.unwrap()->isFull()) {
#ifdef VERDANT_FLAG_DEBUG
        std::cout << "[DEBUG] Borrow from previous sibling" << std::endl;
#endif
        auto prevSibling = optionalPrevSibling.unwrap();
        size_t middleIndex = this->parent->findChildIndex(this) - 1;
        prevSibling->values.push_back(this->parent->values[middleIndex]);
        this->children[0]->parent = prevSibling;
        prevSibling->children.push_back(std::move(this->children[0]));
        this->children.erase(this->children.begin());
        if (insertIndex == 0) {
          this->parent->values[middleIndex] = value;
          ptr->parent = this;
          this->children.insert(this->children.begin(), std::move(ptr));
        } else {
          this->parent->values[middleIndex] = this->values[0];
          this->values.erase(this->values.begin());
          this->values.insert(this->values.begin() + insertIndex - 1, value);
          ptr->parent = this;
          this->children.insert(this->children.begin() + insertIndex, std::move(ptr));
        }
        return std::make_pair(Optional<T>(), nullptr);
      }
    
      // Borrow space from next sibling 
      auto optionalNextSibling = this->parent->getNextChild(this);
      if (optionalNextSibling.unwrappable() && !optionalNextSibling.unwrap()->isFull()) {
#ifdef VERDANT_FLAG_DEBUG
        std::cout << "[DEBUG] Borrow from next sibling" << std::endl;
#endif
        auto nextSibling = optionalNextSibling.unwrap();
        size_t middleIndex = this->parent->findChildIndex(this);
        nextSibling->values.insert(nextSibling->values.begin(), this->parent->values[middleIndex]);
        if (insertIndex == this->values.size()) {
          this->parent->values[middleIndex] = value;
          ptr->parent = nextSibling;
          nextSibling->children.insert(nextSibling->children.begin(), std::move(ptr));
        } else {
          this->parent->values[middleIndex] = this->values[this->values.size() - 1];
          this->values.pop_back();
          this->values.insert(this->values.begin() + insertIndex, value);
          this->children[this->children.size() - 1]->parent = nextSibling;
          nextSibling->children.insert(nextSibling->children.begin(), std::move(this->children[this->children.size() - 1]));
          this->children.pop_back();
          ptr->parent = this;
          this->children.insert(this->children.begin() + insertIndex + 1, std::move(ptr));
        }
    
        return std::make_pair(Optional<T>(), nullptr);
      }
    } else { // this->isLeaf()
      assert(ptr == nullptr);
      // Borrow space from previous sibling
      auto optionalPrevSibling = this->parent->getPrevChild(this);
      if (optionalPrevSibling.unwrappable() && !optionalPrevSibling.unwrap()->isFull()) {
        auto prevSibling = optionalPrevSibling.unwrap();
        size_t middleIndex = this->parent->findChildIndex(this) - 1;
        assert(!(this->values[0] == this->parent->values[middleIndex]) || !(insertIndex == 0));
        if (insertIndex == 0) {
          prevSibling->values.push_back(value);
          this->parent->values[middleIndex] = this->values[0];
        } else { // insertIndex != 0
          prevSibling->values.push_back(this->values[0]);
          this->values.erase(this->values.begin());
          this->values.insert(this->values.begin() + insertIndex - 1, value);
          this->parent->values[middleIndex] = this->values[0];
        }
        return std::make_pair(Optional<T>(), nullptr);
      }
    
      // Borrow space from next sibling 
      auto optionalNextSibling = this->parent->getNextChild(this);
      if (optionalNextSibling.unwrappable() && !optionalNextSibling.unwrap()->isFull()) {
        auto nextSibling = optionalNextSibling.unwrap();
        size_t middleIndex = this->parent->findChildIndex(this);
        if (insertIndex == this->values.size()) {
          nextSibling->values.insert(nextSibling->values.begin(), value);
          this->parent->values[middleIndex] = value;
        } else {
          nextSibling->values.insert(nextSibling->values.begin(), this->values[this->values.size() - 1]);
          this->parent->values[middleIndex] = this->values[this->values.size() - 1];
          this->values.pop_back();
          this->values.insert(this->values.begin() + insertIndex, value);
        }
    
        return std::make_pair(Optional<T>(), nullptr);
      }
    }
  }

  // Node is full. Splitting. Assumming distinct elements
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] Node splitting begin" << std::endl;
  std::unordered_set<T> s(this->values.begin(), this->values.end());
  s.insert(value);
  assert(s.size() == this->values.size() + 1);
#endif
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
#ifdef VERDANT_FLAG_DEBUG
  // Ensure that the median is chosen correctly
  size_t lessThanMedian = std::count_if(this->values.begin(), this->values.end(), [median](T x){return x < median;});
  size_t moreThanMedian = std::count_if(this->values.begin(), this->values.end(), [median](T x){return x > median;});
  if (value < median) {
    lessThanMedian++;
  }
  if (value > median) {
    moreThanMedian++;
  }
  assert(lessThanMedian == moreThanMedian);
#endif
  bool valueAdded = false;
  size_t valuesMoved = 0;
  size_t childrenMoved = 0;
  for (size_t i = this->order - 1; i < 2 * this->order; i++) {
    if (this->values[i] < median) {
      continue;
    }

    if (!valueAdded && value >= median && this->values[i] > value) {
      // Implicit: values[i] > median
      if ((value > median) || (value == median && this->isLeaf())) {
        newNode->values.push_back(value);
      } 
      if (!this->isLeaf()) {
#ifdef VERDANT_FLAG_DEBUG
        std::cout << "[DEBUG] New child pointer added to the new node" << std::endl; 
#endif
        ptr->parent = newNode.get();
        newNode->children.push_back(std::move(ptr));
      }
      valueAdded = true;
    } 

    // Onward: values[i] >= median
    if (!this->isLeaf()) {
      // if values[i] == median -> not add value, and only add the right child
      this->children[i + 1]->parent = newNode.get();
      newNode->children.push_back(std::move(this->children[i + 1]));
      childrenMoved++;
    }

    // this->isLeaf() -> add all, including median
    // non-leaf -> add only if values[i] is not median
    // values[i] added <-> isLeaf() or values[i] > median
    if (this->isLeaf() || this->values[i] > median) {
      newNode->values.push_back(this->values[i]);
      valuesMoved++;
    }
  }

  // Handle when value more than max of values
  if (value > this->values[this->values.size() - 1]) {
    if ((value > median) || (value == median && this->isLeaf())) {
      newNode->values.push_back(value);
    } 
    if (!this->isLeaf()) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[DEBUG] New child pointer added to the new node" << std::endl; 
#endif
      ptr->parent = newNode.get();
      newNode->children.push_back(std::move(ptr));
    }
    valueAdded = true;
  }

  this->values.resize(this->values.size() - valuesMoved);
#ifdef VERDANT_FLAG_DEBUG
  size_t numChildrenOld = this->children.size();
#endif
  if (!this->isLeaf()) {
    this->children.resize(this->children.size() - childrenMoved);
  }
#ifdef VERDANT_FLAG_DEBUG
  if (!this->isLeaf()) {
    assert(childrenMoved == newNode->children.size() - (valueAdded ? 1 : 0));
    assert(this->children.size() + newNode->children.size() + (valueAdded ? -1 : 0) == numChildrenOld);
  }
#endif
  if (!valueAdded) {
    assert(value < median);
    this->values.insert(this->values.begin() + insertIndex, value);
    valueAdded = true;
    if (!this->isLeaf()) {
      ptr->parent = this;
      this->children.insert(this->children.begin() + insertIndex + 1, std::move(ptr));
    }
  }
  if (!this->isLeaf() && this->values[this->values.size() - 1] == median) {
    this->values.pop_back();
  }
#ifdef VERDANT_FLAG_DEBUG
//std::cout << "[DEBUG] Last value of splitted node: " << this->values[this->values.size() - 1] << std::endl; 
//std::cout << "[DEBUG] Upvalue: " << median << std::endl; 
//std::cout << "[DEBUG] First value of new sibling: " << newNode->values[0] << std::endl; 
//std::cout << "[DEBUG] Splitted node ID: " << this->index  << std::endl;
//std::cout << "[DEBUG] Parent ID: " << (this->parent != nullptr ? std::to_string(this->parent->index) : "null") << std::endl;
//std::cout << "[DEBUG] New node ID: " << newNode->index << std::endl;
  assert(valueAdded);
  assert(this->parent == newNode->parent);
  assert(this->values[this->values.size() - 1] < median);
  assert(median <= newNode->values[0]);
  std::cout << "[DEBUG] Node splitting finished" << std::endl;
#endif
  return std::make_pair(Optional<T>(median), std::move(newNode));
}

// Order 2
// 1, 3, 5, 7
// Add 0 -> median = 3
// Add 2 -> index 1 -> median = [2 - 1] = [1] -> 3
// Add 4 -> index = 2 -> order = index -> median = 4
// Add 6 -> index = 3 -> median = [2] = 5
// Add 8 -> median = 5

template <typename T> std::pair<Optional<T>, Optional<size_t>> BTreeNode<T>::remove(const T& value) {
  if (this->isLeaf()) {
    return this->removeOnLeaf(value);
  } else {
    return this->removeOnInternal(value);
  }
}

template <typename T> std::pair<Optional<T>, Optional<size_t>> BTreeNode<T>::removeOnLeaf(const T& value) {
  size_t insertIndex = this->insertIndexSearch(value);
  if (value != this->values[insertIndex]) {
    return std::make_pair(Optional<T>(), Optional<size_t>());
  }
  T result = this->values[insertIndex];
  this->values.erase(this->values.begin() + insertIndex);
  if (this->isRoot() || this->values.size() >= this->order) {
    return std::make_pair(result, Optional<size_t>());
  }
  // NOTE: Least amount of values posisble
#ifdef VERDANT_FLAG_DEBUG
  assert(this->values.size() == this->order - 1);
#endif
  auto optionalPrevSibling = this->parent->getPrevChild(this);
  auto optionalNextSibling = this->parent->getNextChild(this);

  if (optionalPrevSibling.unwrappable() && !optionalPrevSibling.unwrap()->isLeast()) {
    size_t middleIndex = this->parent->findChildIndex(this) - 1;
    auto prevSibling = optionalPrevSibling.unwrap();
    T value = prevSibling->values[prevSibling->values.size() - 1];
    prevSibling->values.pop_back();
    this->parent->values[middleIndex] = value;
    this->values.insert(this->values.begin(), value);
    return std::make_pair(result, Optional<size_t>());
  }
  if (optionalNextSibling.unwrappable() && !optionalNextSibling.unwrap()->isLeast()) {
    size_t middleIndex = this->parent->findChildIndex(this);
    auto nextSibling = optionalNextSibling.unwrap();
    T value = nextSibling->values[0];
    nextSibling->values.erase(nextSibling->values.begin());
    this->values.push_back(value);
    this->parent->values[middleIndex] = nextSibling->values[0];
    return std::make_pair(result, Optional<size_t>());
  }

  if (optionalPrevSibling.unwrappable()) {
    size_t middleIndex = this->parent->findChildIndex(this) - 1;
    auto prevSibling = optionalPrevSibling.unwrap();
    BTreeNode<T>::mergeLeafNodes(prevSibling, this);
    return std::make_pair(result, middleIndex);
  }

  if (optionalNextSibling.unwrappable()) {
    size_t middleIndex = this->parent->findChildIndex(this);
    auto nextSibling = optionalNextSibling.unwrap();
    BTreeNode<T>::mergeLeafNodes(this, nextSibling);
    return std::make_pair(result, middleIndex);
  }
  // Unreachable
#ifdef VERDANT_FLAG_DEBUG
  std::cerr << "[ERROR] Leaf has no sibling" << std::endl;
#endif
  exit(VerdantStatus::INTERNAL_ERROR);
}

template <typename T> std::pair<Optional<T>, Optional<size_t>> BTreeNode<T>::removeOnInternal(const T& value) {
  size_t insertIndex = this->insertIndexSearch(value);
  size_t childIndex = insertIndex + (insertIndex < this->values.size() && this->values[insertIndex] == value ? 1 : 0);
  auto recursiveResult = this->children[childIndex]->remove(value);
  auto result = std::move(recursiveResult.first);
  auto optionalRemovedIndex = std::move(recursiveResult.second);
  if (!optionalRemovedIndex.unwrappable()) {
    return recursiveResult;
  }

  size_t removedIndex = optionalRemovedIndex.unwrap();

  this->values.erase(this->values.begin() + removedIndex);
  this->children.erase(this->children.begin() + removedIndex + 1);

  if (this->isRoot() || this->values.size() >= this->order) {
    return std::make_pair(std::move(result), Optional<size_t>());
  }

  // NOTE: Least amount of values possible
#ifdef VERDANT_FLAG_DEBUG
  assert(this->values.size() == this->order - 1);
#endif
  auto optionalPrevSibling = this->parent->getPrevChild(this);
  auto optionalNextSibling = this->parent->getNextChild(this);

  if (optionalPrevSibling.unwrappable() && !optionalPrevSibling.unwrap()->isLeast()) {
    size_t middleIndex = this->parent->findChildIndex(this) - 1;
    auto prevSibling = optionalPrevSibling.unwrap();
    this->values.insert(this->values.begin(), this->parent->values[middleIndex]);
    this->parent->values[middleIndex] = prevSibling->values[prevSibling->values.size() - 1];
    prevSibling->values.pop_back();
    this->children.insert(this->children.begin(), std::move(prevSibling->children[prevSibling->children.size() - 1]));
    this->children[0]->parent = this;
    prevSibling->children.pop_back();
    return std::make_pair(std::move(result), Optional<size_t>());
  }
  if (optionalNextSibling.unwrappable() && !optionalNextSibling.unwrap()->isLeast()) {
    size_t middleIndex = this->parent->findChildIndex(this);
    auto nextSibling = optionalNextSibling.unwrap();
    this->values.push_back(this->parent->values[middleIndex]);
    this->parent->values[middleIndex] = nextSibling->values[0];
    nextSibling->values.erase(nextSibling->values.begin());
    this->children.push_back(std::move(nextSibling->children[0]));
    this->children[this->children.size() - 1]->parent = this;
    nextSibling->children.erase(nextSibling->children.begin());
    return std::make_pair(std::move(result), Optional<size_t>());
  }

  // Merging
  if (optionalPrevSibling.unwrappable()) {
    size_t middleIndex = this->parent->findChildIndex(this) - 1;
    auto prevSibling = optionalPrevSibling.unwrap();
    mergeInternalNodes(prevSibling, this->parent->values[middleIndex], this);
    return std::make_pair(std::move(result), middleIndex);
  }

  if (optionalNextSibling.unwrappable()) {
    size_t middleIndex = this->parent->findChildIndex(this);
    auto nextSibling = optionalNextSibling.unwrap();
    mergeInternalNodes(this, this->parent->values[middleIndex], nextSibling);
    return std::make_pair(std::move(result), middleIndex);
  }
  // Unreachable
#ifdef VERDANT_FLAG_DEBUG
  std::cerr << "[ERROR] Internal node has no sibling" << std::endl;
#endif
  exit(VerdantStatus::INTERNAL_ERROR);
}

template <typename T> void BTreeNode<T>::mergeLeafNodes(BTreeNode<T>* first, BTreeNode<T>* second) {
  assert(first->next == second);
  for (auto value: second->values) {
    first->values.push_back(value);
  }
  first->next = second->next;
}

template <typename T> void BTreeNode<T>::mergeInternalNodes(BTreeNode<T>* first, const T middleValue, BTreeNode<T>* second) {
  first->values.push_back(middleValue);
  for (auto value: second->values) {
    first->values.push_back(value);
  }
  for (size_t i = 0; i < second->children.size(); i++) {
    second->children[i]->parent = first;
    first->children.push_back(std::move(second->children[i]));
  }
}

template <typename T> Optional<T> BTreeNode<T>::search(const T& value) {
  if (this->values.size() == 0) {
    return Optional<T>();
  }

  size_t insertIndex = this->insertIndexSearch(value);

  if (insertIndex != this->values.size() && this->values[insertIndex] == value) {
    if (this->isLeaf()) {
      return this->values[insertIndex];
    }
    return this->children[insertIndex + 1]->search(value);
  } else {
    if (this->isLeaf()) {
      return Optional<T>();
    }
#ifdef VERDANT_FLAG_DEBUG
    assert(insertIndex == this->values.size() || this->values[insertIndex] > value);
    assert(insertIndex == 0 || this->values[insertIndex - 1] < value);
#endif

    return this->children[insertIndex]->search(value);
  }
}

template <typename T>  Optional<std::vector<T>> BTreeNode<T>::searchRange(const T& minVal, const T& maxVal) {
  size_t insertIndex = this->insertIndexSearch(minVal);
  if (this->isLeaf()) {
    std::vector<T> rangeResult;
    BTreeNode<T>* curNode = this;
    size_t curIndex = insertIndex;
    while (curNode != nullptr) {
      if (curIndex == curNode->values.size()) {
        curIndex = 0;
        curNode = curNode->next;
        continue;
      }
      if (curNode->values[curIndex] > maxVal) {
        break;
      }
      rangeResult.push_back(curNode->values[curIndex]);
      curIndex++;
    }
    return rangeResult;
  }
  if (insertIndex != this->values.size() && this->values[insertIndex] == minVal) {
    return this->children[insertIndex + 1]->searchRange(minVal, maxVal);
  } else {
    return this->children[insertIndex]->searchRange(minVal, maxVal);
  }
}

template <typename T> std::pair<bool, size_t> BTreeNode<T>::validate(bool root, BTreeNode* parent, T* minVal, T* maxVal) {
  // NOTE: Verify that the parent information is correct
  if (this->parent != parent) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] Mismatching parent pointers." << std::endl;
    std::cout << "[DEBUG] Current node ID: " << this->index << std::endl;
    std::cout << "[DEBUG] Parent node ID: " << parent->index << std::endl;
    std::cout << "[DEBUG] Currently stored parent node ID: " << this->parent->index << std::endl;
#endif
    return { false, SIZE_MAX };
  }
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
    std::cout << "[DEBUG] Problematic node: " << this->index << std::endl;
#endif
    return { false, SIZE_MAX };
  }

  if (this->values.size() > 0) {
    // Check that the first key is larger or equal to minVal 
    if (minVal != nullptr && this->values[0] < *minVal) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[ERROR] BTree failed by having first key of a node less than previous key of its parent node." << std::endl;
      std::cout << "[DEBUG] Current node ID: " << this->index << std::endl;
      std::cout << "[DEBUG] Current node first value: " << this->values[0] << std::endl;
      std::cout << "[DEBUG] Compared value : " << *minVal << std::endl;
#endif
      return { false, SIZE_MAX };
    }

    // Check that the last key is less than maxVal
    if (maxVal != nullptr && this->values[this->values.size() - 1] >= *maxVal) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[ERROR] BTree failed by having last key of a node larger than or equal to previous key of its parent node." << std::endl;
      std::cout << "[DEBUG] Current node ID: " << this->index << std::endl;
      std::cout << "[DEBUG] Parent ID: " << this->parent->index << std::endl;
      std::cout << "[DEBUG] Last value: " << this->values[this->values.size() - 1] << std::endl;
      std::cout << "[DEBUG] Right-parent value: " << *maxVal << std::endl;
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

  // NOTE: Rule 3: All non-leaf node have at least 2 children
  if (!this->isLeaf() && this->children.size() < 2) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] Non-leaf node has less than 2 children" << std::endl;
    std::cout << "[DEBUG] Number of children: " << this->children.size() << std::endl;
    std::cout << "[DEBUG] Node ID: " << this->index << std::endl;
    if (this->isRoot()) {
      std::cout << "[DEBUG] Node is root" << std::endl;
    }
#endif
    return { false, SIZE_MAX };
  }

  size_t childDepth = 0;
  for (size_t i = 0; i < this->children.size(); i++) {
    if (this->children[i] == nullptr) {
#ifdef VERDANT_FLAG_DEBUG
      std::cout << "[ERROR] Null node detected at node ID " << this->index << std::endl;
      std::cout << "[DEBUG] Number of values: " << this->values.size() << std::endl;
      std::cout << "[DEBUG] Number of children: " << this->children.size() << std::endl;
      std::cout << "[DEBUG] Null child index: " << i << std::endl;
      exit(VerdantStatus::INTERNAL_ERROR);
#endif
    }
    std::pair<bool, size_t> childResult = this->children[i]->validate(false, this, (i == 0 ? minVal : &this->values[i - 1]), (i == this->children.size() - 1 ? maxVal : &this->values[i]));
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

  // NOTE: Rule 5: A non-leaf node with n keys has exactly n + 1 children
  if (!this->isLeaf() && this->children.size() != this->values.size() + 1) {
#ifdef VERDANT_FLAG_DEBUG
    std::cout << "[ERROR] Mismatched keys-children values" << std::endl;
    std::cout << "[DEBUG] Number of keys: " << this->values.size() << std::endl;
    std::cout << "[DEBUG] Number of children: " << this->children.size() << std::endl;
#endif
    return { false, SIZE_MAX };
  }

  return { true, 0 };
}


template <typename T> Optional<T> BTreeNode<T>::getMinValue() {
  if (this->isLeaf()) {
    return this->values[0];
  }
  return this->children[0]->getMinValue();
}

template <typename T> Optional<T> BTreeNode<T>::getMaxValue() {
  if (this->isLeaf()) {
    return this->values[this->values.size() - 1];
  }
  return this->children[this->children.size() - 1]->getMaxValue();
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
