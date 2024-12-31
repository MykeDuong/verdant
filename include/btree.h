#pragma once

#include "optional.h"

#include <vector>
#include <deque>
#include <memory>

template <typename T> class BTree;

template <typename T> class BTreeNode{
private:
  size_t order;
  BTreeNode* next;
  std::vector<std::unique_ptr<BTreeNode<T>>> children;
  std::vector<T> values;
  BTreeNode* parent;
  friend class BTree<T>;
#ifdef VERDANT_FLAG_DEBUG
  static size_t nextIndex;
  size_t index;
#endif

  size_t insertIndexSearch(const T& value);

  bool isLeaf();

  bool isRoot();

  bool isFull();

  size_t findChildIndex(BTreeNode<T>* child);

  Optional<BTreeNode<T>*> getPrevChild(BTreeNode* curChild);

  Optional<BTreeNode<T>*> getNextChild(BTreeNode* curChild);

  std::pair<Optional<T>, std::unique_ptr<BTreeNode<T>>> insert(T value);

  BTreeNode(size_t order, BTreeNode* parent = nullptr);

  bool remove(T& value);

  Optional<T> search(const T& value);

  Optional<std::vector<T>> searchRange(const T& minVal, const T& maxVal);

  std::pair<bool, size_t> validate(bool root = false, BTreeNode* parent = nullptr, T* minVal = nullptr, T* maxVal = nullptr);

  Optional<T> getMinValue();

  Optional<T> getMaxValue();

  size_t getHeight();
  
  size_t countNodes();

  size_t countKeys();

};

template <typename T> class BTree {
private:
  size_t order;
  std::unique_ptr<BTreeNode<T>> root;
public:
  BTree();
  ~BTree();

  bool insert(T value);

  bool remove(T& value);

  Optional<T> search(const T& value);

  Optional<std::vector<T>> searchRange(const T& minVal, const T& maxVal);

  bool validate();

  Optional<T> getMinValue();

  Optional<T> getMaxValue();

  size_t getHeight();

  size_t countNodes();

  double getAverageKeyPerNode();
};

#include "btree.cpp"
