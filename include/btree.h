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

  size_t insertIndexSearch(T& value);

  bool isLeaf();

  Optional<BTreeNode<T>*> getPrevChild(BTreeNode* curChild);

  Optional<BTreeNode<T>*> getNextChild(BTreeNode* curChild);

  std::pair<Optional<T>, std::unique_ptr<BTreeNode<T>>> insert(T value);

  BTreeNode(size_t order, BTreeNode* parent = nullptr);

  bool remove(T& value);

  Optional<T> search(T& value);

  std::pair<bool, size_t> validate(bool root = false, T* maxVal = nullptr, T* minVal = nullptr);

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

  Optional<T> search(T& value);

  bool validate();

  size_t getHeight();

  size_t countNodes();

  double getAverageKeyPerNode();
};

#include "btree.cpp"
