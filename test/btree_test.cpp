#include "btree.h"
#include "parameters.h"

#include <climits>
#include <random>

size_t BLOCK_SIZE = 8192;

int main() {
  BTree<int> btree;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<int> uni(0, INT_MAX);

  for (size_t i = 0; i < 10000; i++) {
    int randomInt = uni(rng);
    btree.insert(randomInt);
  }


  return 0;
}
