#include "btree.h"
#include "parameters.h"

#include <climits>
#include <random>

size_t BLOCK_SIZE = 8192;

int main() {
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] Debug enabled" << std::endl;
#endif
  BTree<int> btree;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<int> uni(0, INT_MAX);
  std::cout << "[DEBUG] Start fuzzing" << std::endl;

  for (size_t i = 0; i < 100000; i++) {
    int randomInt = uni(rng);
    btree.insert(randomInt);
    assert(btree.validate());
    std::cout << "[DEBUG] Finished round " << i + 1 << std::endl;
  }

  std::cout << "[DEBUG] End fuzzing" << std::endl;
  std::cout << "[DEBUG] Tree height: " << btree.getHeight() << std::endl;
  std::cout << "[DEBUG] Average keys per node: " << btree.getAverageKeyPerNode() << std::endl;
  std::cout << "[DEBUG] Number of nodes: " << btree.countNodes() << std::endl;

  return 0;
}
